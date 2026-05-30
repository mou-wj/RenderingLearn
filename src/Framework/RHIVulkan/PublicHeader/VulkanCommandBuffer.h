#pragma once
#include "VulkanDevice.h"
#include "VulkanResource.h"
#include "VulkanSync.h"
#include "VulkanBarriers.h"
#include <vector>
#include <memory>
#include <unordered_map>


namespace RHIVulkan
{

class VulkanCommandContext; // 前向声明
class VulkanDevice;

class VulkanCommandBufferPool;

class VulkanCommandBuffer
{
public:
    VulkanCommandBuffer(VulkanDevice* device, VulkanCommandBufferPool* owner,VkCommandBufferLevel level);
    ~VulkanCommandBuffer();

    enum ECommandBufferState {
        Free,//空闲
        Pending,//已经提交
        NeedRecycle//需要回收
    };

    void MarkState(ECommandBufferState state) { this->state = state; }
    ECommandBufferState GetState() const { return state; }

    VkCommandBuffer GetHandle() const { return commandBuffer; }
    void AllocateMemory();

    void Begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    void End();

    void Reset();


	VulkanImageLayoutManager* GetImageLayoutManager() { return &imageLayoutManager; }

private:
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VulkanCommandBufferPool* owner = nullptr;
    VulkanDevice* device = nullptr;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    friend class VulkanQueue;
    VulkanImageLayoutManager imageLayoutManager;
    ECommandBufferState state = ECommandBufferState::Free;
};

class VulkanCommandBufferPool
{
public:
    VulkanCommandBufferPool(VulkanDevice* device, uint32_t queueFamilyIndex);
    ~VulkanCommandBufferPool();

    VkCommandPool GetHandle() const { return commandPool; }

    // 分配一个 CommandBuffer
    VulkanCommandBuffer* AllocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Reset pool，所有 buffer 都会被重置
    void Reset();

private:
    VulkanDevice* device = nullptr;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // 已分配但未被使用的 buffer（空闲池）
    std::vector<VulkanCommandBuffer*> availableBuffers;

    // 所有 buffer 管理，方便析构统一释放
    std::vector<std::unique_ptr<VulkanCommandBuffer>> allBuffers;

    uint32_t queueFamilyIndex = 0;
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
    VulkanCommandBuffer* GetActiveCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VulkanCommandBuffer* EndActiveCommandBuffer();


    // 重置所有池
    void Reset();

    void GarbageCollect();

private:
    // 分配命令缓冲区
    VulkanCommandBuffer* Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VulkanCommandBuffer* ActiveCommandBuffer = nullptr;
    VulkanDevice* device = nullptr;
    VulkanCommandContext* commandContext = nullptr;
    std::unordered_map<uint32_t, std::unique_ptr<VulkanCommandBufferPool>> Pools;
    std::vector<VulkanCommandBuffer*> ManagedBuffers; // 管理所有分配的buffer
};



} // namespace WR::RHIVulkan

