#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommandContex.h"
#include "VulkanQueue.h"
#include <stdexcept>

namespace RHIVulkan{

// VulkanCommandBuffer

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VkCommandBuffer cmdBuffer)
    : device(device), commandBuffer(cmdBuffer)
{}

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

// VulkanCommandBufferPool

VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice* device, uint32_t queueFamilyIndex)
    : device(device)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device->GetDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device->GetDevice(), commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
}

VulkanCommandBuffer* VulkanCommandBufferPool::AllocateCommandBuffer(VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    if (vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }

    return new VulkanCommandBuffer(device, cmdBuffer);
}

void VulkanCommandBufferPool::Reset()
{
    vkResetCommandPool(device->GetDevice(), commandPool, 0);
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

VulkanCommandBuffer* VulkanCommandBufferManager::GetAvailableCommandBuffer(VkCommandBufferLevel level)
{
    // 查找可复用的命令缓冲区（Fence已完成）
    for (auto& buffer : ManagedBuffers)
    {
        VulkanFence* fence = buffer->GetFence();
        if (fence && fence->IsSignaled())
        {
            buffer->Reset();
            return buffer.get();
        }
    }
    // 没有可用的，分配新的
    VulkanCommandBuffer* newBuffer = Allocate(level);
    ManagedBuffers.emplace_back(newBuffer);
    return newBuffer;
}

void VulkanCommandBufferManager::Reset()
{
    for (auto& pair : Pools)
    {
        pair.second->Reset();
    }
}

} // namespace WR::RHIVulkan