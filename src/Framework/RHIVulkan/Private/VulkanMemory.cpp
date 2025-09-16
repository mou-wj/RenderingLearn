#include "VulkanMemory.h"
#include "VulkanDevice.h"
#include <stdexcept>
#include <algorithm>
#include <cstring>
namespace RHIVulkan{
    
constexpr VkDeviceSize DEFAULT_BLOCK_SIZE = 64 * 1024 * 1024; // 64 MB

// VulkanMemoryBlock
VulkanMemoryBlock::VulkanMemoryBlock(VulkanDevice* device, uint32_t memoryTypeIndex, VkDeviceSize blockSize, bool mappable)
    : device_(device), memoryTypeIndex_(memoryTypeIndex), size_(blockSize), offset_(0), mappable_(mappable), mapped_(nullptr)
{
    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = size_;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(device_->GetDevice(), &allocInfo, nullptr, &memory_) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate Vulkan memory block");

    if (mappable_)
        vkMapMemory(device_->GetDevice(), memory_, 0, VK_WHOLE_SIZE, 0, &mapped_);
}

VulkanMemoryBlock::~VulkanMemoryBlock()
{
    if (mapped_)
        vkUnmapMemory(device_->GetDevice(), memory_);
    vkFreeMemory(device_->GetDevice(), memory_, nullptr);
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
    vkGetPhysicalDeviceMemoryProperties(device_->GetPhysicalDevice(), &memProps);

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
    auto newBlock = std::make_unique<VulkanMemoryBlock>(device_, typeIndex, std::max(DEFAULT_BLOCK_SIZE, memReqs.size), (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);
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
}