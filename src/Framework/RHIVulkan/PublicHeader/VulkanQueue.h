#pragma once

#include "RHICommandContex.h"
#include "VulkanDevice.h"
#include "VulkanBarriers.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSync.h"
#include <vector>
#include <memory>
#include <mutex>

namespace RHIVulkan{

class VulkanDevice;
class VulkanSemaphore;
class VulkanCommandContext;

class RHIVULKAN_API VulkanQueue : public RHI::RHIQueue
{
public:
    VulkanQueue() = default;
    VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex, EQueueType type
    );
    ~VulkanQueue() override;

    // Initialize the context pool for this queue. poolSize contexts will be pre-created.
    void InitContextPool(uint32_t poolSize = 1);

    VkQueue GetHandle() const { return queue_; }
    uint32_t GetFamilyIndex() const { return familyIndex_; }
    VulkanDevice* GetDevice() const { return device_; }
    VulkanImageLayoutManager* GetImageLayoutManager() { return &imageLayoutManager_; }
    void UpdatedCommandBufferImageLayoutManager(VulkanCommandBuffer* commandBuffer) {
        if (commandBuffer) {
            auto layoutManager = commandBuffer->GetImageLayoutManager();
            imageLayoutManager_.TransferTo(*layoutManager);
        }
    }

    // ---- RHI::RHIQueue overrides ----
    RHI::EQueueType GetType() const override;
    RHI::RHIContextBase* AcquireCommandContext() override;
    RHI::RHIContextBase* ReleaseCommandContext(RHI::RHIContextBase* Context) override;
    RHI::RHIFence ExecuteContext(RHI::RHIContextBase* context) override;
    RHI::RHIFence ExecuteContext(const std::vector<RHI::RHIContextBase*>& Cmds, const std::vector<RHI::RHIWaitInfo>& WaitInfos) override;
    void SubmitEmptyWithDependency(VkSemaphore timelineWait, uint64_t waitValue, VkSemaphore binarySignal);
    void WaitFence(RHIFence Fence) override;

    void WaitIdle() override;

    void GarbageCollect();

private:
    struct PendingInfo {
        std::vector<VulkanCommandBuffer*> Cmds;
        uint64_t FinishedTimelineValue;
    };
    std::queue<PendingInfo> PendingInfos;

    VulkanDevice* device_ = nullptr;
    VkQueue queue_ = VK_NULL_HANDLE;
    uint32_t familyIndex_ = UINT32_MAX;
    VulkanImageLayoutManager imageLayoutManager_;
    RHI::EQueueType QueueType_ = RHI::EQueueType::Graphics;
    std::vector<VulkanCommandContext*> AllContexts_;  // owns all pooled contexts
    std::vector<VulkanCommandContext*> FreeContexts_; // contexts available for acquire
    VulkanRHISyncPoint* SubmitSyncPoint_ = nullptr; // 用于跨提交等待的时间线信号量
    uint64_t currentTimelineValue_ = 0;
    std::mutex ContextPoolMutex_;
    std::mutex FlushContextMutex_;
};

// -------------------------------------------------------------------------------------------------
// Present executor backed by a VulkanQueue (graphics queue)
// -------------------------------------------------------------------------------------------------
class RHIVULKAN_API VulkanPresentExecutor final : public RHI::RHIPresentExecutor
{
public:
    explicit VulkanPresentExecutor(VulkanQueue* queue);
    void Present(RHI::RHISwapchain* Swapchain, const RHI::RHIWaitInfo& Waitinfo) override;
private:
    VulkanQueue* Queue = nullptr;
};

}