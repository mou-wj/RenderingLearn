#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommandContex.h"
#include "VulkanQueue.h"
#include "VulkanFuncWrapper.h"
#include <stdexcept>

namespace RHIVulkan{

// VulkanCommandBuffer



VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VulkanCommandBufferPool* owner, VkCommandBufferLevel level) :device(device), owner(owner),level(level)
{
    fence = device->GetFenceManager()->AcquireFence();
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    device->GetFenceManager()->ReleaseFence(fence);
}

void VulkanCommandBuffer::AllocateMemory()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = owner->GetHandle();
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if (!AllocateCommandBuffers(device->GetHandle(), &allocInfo, &commandBuffer)) {
        throw std::runtime_error("Failed to allocate command buffer");
    }
}

void VulkanCommandBuffer::Begin(VkCommandBufferUsageFlags usage)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin command buffer");
    }

}

void VulkanCommandBuffer::End()
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to end command buffer");
    }
}

void VulkanCommandBuffer::Reset()
{
    vkResetCommandBuffer(commandBuffer, 0);
}

void VulkanCommandBuffer::AddWaitSemaphores(VkPipelineStageFlags stage, const std::vector<VulkanSemaphore*>& semaphores)
{
    if (semaphores.empty())
    {
        return;
    }
    for (auto* semaphore : semaphores)
    {
        WaitFlags.push_back(stage);
        WaitSemaphores.push_back(semaphore);
    }
}

void VulkanCommandBuffer::AddSignalSemaphores(const std::vector<VulkanSemaphore*>& semaphores)
{
    if (semaphores.empty())
    {
        return;
    }
    SignalSemaphores.insert(SignalSemaphores.end(), semaphores.begin(), semaphores.end());
}

// VulkanCommandBufferPool
VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice* device, uint32_t queueFamily)
    : device(device), queueFamilyIndex(queueFamily)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // 支持单个或整体 reset

    vkCreateCommandPool(device->GetHandle(), &poolInfo, nullptr, &commandPool);
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
    // 析构所有 CommandBuffer
    allBuffers.clear();

    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device->GetHandle(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}

VulkanCommandBuffer* VulkanCommandBufferPool::AllocateCommandBuffer(VkCommandBufferLevel level)
{
    // 先从可用池拿
    if (!availableBuffers.empty())
    {
        auto buf = availableBuffers.back();
        availableBuffers.pop_back();
        return buf;
    }

    auto cmdBuf = std::make_unique<VulkanCommandBuffer>(device, this,level);
    cmdBuf->AllocateMemory();
    VulkanCommandBuffer* ptr = cmdBuf.get();

    allBuffers.push_back(std::move(cmdBuf));
    return ptr;
}

void VulkanCommandBufferPool::Reset()
{
    // 重置 Vulkan CommandPool，所有 CommandBuffer 自动重置
    vkResetCommandPool(device->GetHandle(), commandPool, 0);

    // 所有 buffer 都回到可用池
    availableBuffers.clear();
    for (auto& buf : allBuffers) // 注意 auto&
    {
        availableBuffers.push_back(buf.get());
    }
}

// VulkanCommandBufferManager

VulkanCommandBufferManager::VulkanCommandBufferManager(VulkanDevice* device, VulkanCommandContext* commandContext)
    : device(device), commandContext(commandContext)
{
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
    Pools.clear();
}

VulkanCommandBufferPool* VulkanCommandBufferManager::GetPool()
{
    // 假设每个Manager只管理一个队列族，由commandContext->GetQueue()获得
    uint32_t queueFamilyIndex = commandContext->GetQueue()->GetFamilyIndex();
    auto it = Pools.find(queueFamilyIndex);
    if (it != Pools.end())
    {
        return it->second.get();
    }
    // 创建新的池
    auto pool = std::make_unique<VulkanCommandBufferPool>(device, queueFamilyIndex);
    VulkanCommandBufferPool* poolPtr = pool.get();
    Pools[queueFamilyIndex] = std::move(pool);
    return poolPtr;
}

VulkanCommandBuffer* VulkanCommandBufferManager::Allocate(VkCommandBufferLevel level)
{
    VulkanCommandBufferPool* pool = GetPool();
    return pool->AllocateCommandBuffer(level);
}

VulkanCommandBuffer* VulkanCommandBufferManager::GetActiveCommandBuffer(VkCommandBufferLevel level)
{
    if (ActiveCommandBuffer) {
		return ActiveCommandBuffer;
    }

    // 查找可复用的命令缓冲区（Fence已完成）
    for (auto& buffer : ManagedBuffers)
    {
        VulkanFence* fence = buffer->GetFence();
        if (fence && fence->IsSignaled())
        {
            fence->Reset();
            buffer->Reset();
            buffer->Begin();
            commandContext->GetQueue()->UpdatedCommandBufferImageLayoutManager(buffer);
            ActiveCommandBuffer = buffer;
            return buffer;
        }
    }
    // 没有可用的，分配新的
    VulkanCommandBuffer* newBuffer = Allocate(level);
    ManagedBuffers.emplace_back(newBuffer);
    newBuffer->Begin();
    commandContext->GetQueue()->UpdatedCommandBufferImageLayoutManager(newBuffer);
    ActiveCommandBuffer = newBuffer;
    return newBuffer;
}

void VulkanCommandBufferManager::SubmitActiveCommandBuffer(uint32_t NumSignalSemaphores, VulkanSemaphore* SignalSemaphores)
{
	VulkanCommandBuffer* commandBuffer = EndActiveCommandBuffer();
	if (!commandBuffer) return;

	commandContext->GetQueue()->SubmitCommandBuffer(commandBuffer, NumSignalSemaphores, SignalSemaphores);
	// ⚠️ 不立刻 Reset，等 Fence 完成后由外部回收
}

VulkanCommandBuffer* VulkanCommandBufferManager::EndActiveCommandBuffer()
{
    if (!ActiveCommandBuffer)
    {
        return nullptr;
    }

    ActiveCommandBuffer->End();
    VulkanCommandBuffer* completedCommandBuffer = ActiveCommandBuffer;
    ActiveCommandBuffer = nullptr;
    return completedCommandBuffer;
}

void VulkanCommandBufferManager::Reset()
{
    for (auto& pair : Pools)
    {
        pair.second->Reset();
    }
}

VulkanCommandBuffer* VulkanCommandBufferManager::BeginUploadCommandBuffer()
{
    if (ActiveUploadCommandBuffer)
        return ActiveUploadCommandBuffer;

    ActiveUploadCommandBuffer = Allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    ActiveUploadCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    commandContext->GetQueue()->UpdatedCommandBufferImageLayoutManager(ActiveUploadCommandBuffer);
    return ActiveUploadCommandBuffer;
}

void VulkanCommandBufferManager::EndAndSubmitUploadCommandBuffer(VulkanCommandBuffer* cmd)
{
    if (!cmd) return;

    cmd->End();
    
    commandContext->GetQueue()->SubmitCommandBuffer(cmd);

    // ⚠️ 不立刻 Reset，等 Fence 完成后由外部回收
    ActiveUploadCommandBuffer = nullptr;
}

void VulkanCommandBufferManager::GarbageCollect()
{
	for (auto it = ManagedBuffers.begin(); it != ManagedBuffers.end();)
	{
		VulkanFence* fence = (*it)->GetFence();
		if (fence && fence->IsSignaled())
		{
			// 回收资源
			(*it)->Reset();
			it = ManagedBuffers.erase(it);
		}
		else
		{
			++it;
		}
	}
}

} // namespace WR::RHIVulkan