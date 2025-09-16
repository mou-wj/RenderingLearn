#pragma once
#include "VulkanDevice.h"
#include "VulkanResource.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace RHIVulkan
{

class VulkanCommandContext; // 前向声明
class VulkanDevice;


class VulkanCommandBuffer
{
public:
    VulkanCommandBuffer(VulkanDevice* device, VkCommandBuffer cmdBuffer);
    ~VulkanCommandBuffer() = default;

    VkCommandBuffer GetHandle() const { return commandBuffer; }

    void Begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    void End();

    void Reset();
    VulkanFence* GetFence() const { return fence; } // 可选：用于同步
private:
    VulkanDevice* device = nullptr;
    VulkanFence* fence = nullptr; // 可选：用于同步
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

class VulkanCommandBufferPool
{
public:
    VulkanCommandBufferPool(VulkanDevice* device, uint32_t queueFamilyIndex);
    ~VulkanCommandBufferPool();

    VkCommandPool GetHandle() const { return commandPool; }

    VulkanCommandBuffer* AllocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void Reset();

private:
    VulkanDevice* device = nullptr;
    VkCommandPool commandPool = VK_NULL_HANDLE;
};

// 新增：VulkanCommandBufferManager
class VulkanCommandBufferManager
{
public:
    VulkanCommandBufferManager(VulkanDevice* device, VulkanCommandContext* commandContext);
    ~VulkanCommandBufferManager();

    // 获取指定队列族的池
    VulkanCommandBufferPool* GetPool();



    // 获取一个可用的命令缓冲区（如有空闲则复用，否则新分配）
    VulkanCommandBuffer* GetAvailableCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // 重置所有池
    void Reset();

private:
    // 分配命令缓冲区
    VulkanCommandBuffer* Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VulkanDevice* device = nullptr;
    VulkanCommandContext* commandContext = nullptr;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanCommandBufferPool>> Pools;
    std::vector<std::unique_ptr<VulkanCommandBuffer>> ManagedBuffers; // 管理所有分配的buffer
};



} // namespace WR::RHIVulkan

