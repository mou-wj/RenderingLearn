#include "VulkanMemory.h"
#include "VulkanDevice.h"
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cstring>
#include "VulkanCommandBuffer.h"
#include "VulkanSync.h"
#include "VulkanFuncWrapper.h"
namespace RHIVulkan{
   
constexpr VkDeviceSize DEFAULT_BLOCK_SIZE = 64 * 1024 * 1024; // 64 MB

// VulkanMemoryBlock
VulkanMemoryBlock::VulkanMemoryBlock(VulkanDevice* device, uint32_t memoryTypeIndex, VkDeviceSize blockSize, bool mappable)
    : device_(device), memoryTypeIndex_(memoryTypeIndex), size_(blockSize), offset_(0), mappable_(mappable), mapped_(nullptr)
{
    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = size_;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (!VKFunc::AllocateMemory(device_->GetHandle(), &allocInfo, &memory_))
        throw std::runtime_error("Failed to allocate Vulkan memory block");

    if (mappable_)
        VKFunc::MapMemory(device_->GetHandle(), memory_, 0, VK_WHOLE_SIZE, 0, &mapped_);
}

VulkanMemoryBlock::~VulkanMemoryBlock()
{
    if (mapped_)
        VKFunc::UnmapMemory(device_->GetHandle(), memory_);
    VKFunc::FreeMemory(device_->GetHandle(), memory_);
}

bool VulkanMemoryBlock::Allocate(VkDeviceSize size, VkDeviceSize alignment, VulkanAllocation& outAlloc)
{
    VkDeviceSize alignedOffset = (offset_ + alignment - 1) & ~(alignment - 1);
    if (alignedOffset + size > size_)
        return false;

    outAlloc = VulkanAllocation(memory_, alignedOffset, size, mapped_ ? static_cast<char*>(mapped_) + alignedOffset : nullptr);
    offset_ = alignedOffset + size;
    return true;
}

void VulkanMemoryBlock::Reset()
{
    offset_ = 0;
}

bool VulkanMemoryBlock::Free(VulkanAllocation& alloc)
{
    if (alloc.GetMemory() == memory_)
    {
        alloc = VulkanAllocation(); // 清空分配
        return true;
    }
    return false;
}


// VulkanMemoryManager
VulkanMemoryManager::VulkanMemoryManager(VulkanDevice* device)
    : device_(device) {}

VulkanMemoryManager::~VulkanMemoryManager()
{
    blocks_.clear();
}

uint32_t VulkanMemoryManager::FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProps;
    VKFunc::GetPhysicalDeviceMemoryProperties(device_->GetPhysicalDevice(), &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
    {
        if ((typeBits & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

bool VulkanMemoryManager::Allocate(const VkMemoryRequirements& memReqs, VkMemoryPropertyFlags props, VulkanAllocation& outAlloc)
{
    uint32_t typeIndex = FindMemoryType(memReqs.memoryTypeBits, props);
    for (auto& block : blocks_)
    {
        if (block->GetMemoryTypeIndex() == typeIndex)
        {
            if (block->Allocate(memReqs.size, memReqs.alignment, outAlloc))
                return true;
        }
    }

    // Allocate new block
    auto newBlock = std::make_unique<VulkanMemoryBlock>(device_, typeIndex, max(DEFAULT_BLOCK_SIZE, memReqs.size), (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);
    bool success = newBlock->Allocate(memReqs.size, memReqs.alignment, outAlloc);
    if (!success)
        return false;

    blocks_.push_back(std::move(newBlock));
    return true;
}

void VulkanMemoryManager::Reset()
{
    for (auto& block : blocks_)
        block->Reset();
}

bool VulkanMemoryManager::Free( VulkanAllocation& alloc)
{
    if (alloc.GetMemory() == VK_NULL_HANDLE)
        return true;

    // 找到分配所在的内存块
    for (auto& block : blocks_)
    {
        if (block->Free(alloc))
        {
            alloc = VulkanAllocation();
            return true;
        }
    }
    return false;
}



VulkanStagingBuffer::VulkanStagingBuffer(VulkanDevice* device, VulkanMemoryManager* memManager, VkDeviceSize size)
    : RHI::RHIStagingBuffer(size), device_(device), memManager_(memManager), size_(size)
{
    // 创建 buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size_;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


    VKFunc::CreateBuffer(device_->GetHandle(), &bufferInfo, &buffer_);
#ifdef DEBUG_INFO
    std::string debugName;
    char buf[64];
    snprintf(buf, sizeof(buf), "VulkanStagingBuffer:0x%llx", (unsigned long long)buffer_);
    debugName = buf;
	VKFunc::SetDebugName(device_->GetHandle(), VK_OBJECT_TYPE_BUFFER, (uint64_t)buffer_, debugName.c_str());
#endif

    // 查询 memory requirements
    VkMemoryRequirements memReq;
    VKFunc::GetBufferMemoryRequirements(device_->GetHandle(), buffer_, &memReq);


    // 分配 HostVisible 内存
    memManager_->Allocate(memReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, allocation_);


    VKFunc::BindBufferMemory(device_->GetHandle(), buffer_, allocation_.GetMemory(), allocation_.GetOffset());


    // 映射内存
    mapped_ = allocation_.GetMappedPointer();
}


VulkanStagingBuffer::~VulkanStagingBuffer()
{
    if (buffer_)
    {
        device_->EnqueueBufferForDeletion(buffer_);
        buffer_ = VK_NULL_HANDLE;
    }


    // VulkanMemoryManager 可以管理释放 allocation，或者这里不做事
}
void* VulkanStagingBuffer::Map(uint32_t Offset, uint32_t NumBytes)
{
    return mapped_;
}
void VulkanStagingBuffer::Unmap()
{

}
VulkanStagingManager::VulkanStagingManager(VulkanDevice* device, VulkanMemoryManager* memManager)
    : device_(device), memManager_(memManager)
{
}


VulkanStagingManager::~VulkanStagingManager()
{
    freeBuffers_.clear();
}


std::shared_ptr<VulkanStagingBuffer> VulkanStagingManager::Acquire(VkDeviceSize size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = freeBuffers_.begin(); it != freeBuffers_.end(); ++it)
    {
        if ((*it)->GetSize() >= size)
        {
            auto buf = *it;
            freeBuffers_.erase(it);
            return buf;
        }
    }

    return std::make_shared<VulkanStagingBuffer>(device_, memManager_, size);
}

void VulkanStagingManager::ReleaseToCmdBuffer(
    VulkanCommandBuffer* cmd,
    std::shared_ptr<VulkanStagingBuffer> buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto& entry = pendingBufferMap_[cmd];
    entry.pendingBuffers.push_back(buffer);
}

void VulkanStagingManager::MarkMappedBuffersUsed(std::shared_ptr<VulkanStagingBuffer> buffer, bool used)
{
	std::lock_guard<std::mutex> lock(mutex_);
	mapedToFreeBufferUsed_[buffer] = used;
}

void VulkanStagingManager::GarbageCollect()
{
    std::lock_guard<std::mutex> lock(mutex_);
	std::vector<VulkanCommandBuffer*> completedCmds;
	std::vector<std::shared_ptr<VulkanStagingBuffer>> buffersToFree; 
	for (auto& entry : pendingBufferMap_)
	{
		if (entry.first->GetState() == VulkanCommandBuffer::NeedRecycle)
		{
			completedCmds.push_back(entry.first);
			for (auto& buf : entry.second.pendingBuffers)
			{
                if (mapedToFreeBufferUsed_.find(buf) != mapedToFreeBufferUsed_.end()) {
                    if (mapedToFreeBufferUsed_[buf] = true) {
                        mapedToFreeBuffers_.push_back(buf);
                    }
                    else {
                        freeBuffers_.push_back(buf);
                        mapedToFreeBufferUsed_.erase(buf);
                    }
                    continue;
                }
                freeBuffers_.push_back(buf);
			}
		}
	}
    for (auto& buf : mapedToFreeBuffers_) {
        if (mapedToFreeBufferUsed_.find(buf) != mapedToFreeBufferUsed_.end()) {
            if (mapedToFreeBufferUsed_[buf] = false) {
                freeBuffers_.push_back(buf);
                mapedToFreeBufferUsed_.erase(buf);
            }
        }
    }
    for (auto& cmd : completedCmds)
	{
        pendingBufferMap_.erase(cmd);
	}

}

}