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
    VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex);
    ~VulkanQueue() override;

    // Initialize the context pool for this queue. poolSize contexts will be pre-created.
    void InitContextPool(RHI::EQueueType type, uint32_t poolSize = 1);

    // Low-level Vulkan submit (used internally by command buffer manager)
    void SubmitCommandBuffer(VulkanCommandBuffer* CmdBuffer, uint32_t NumSignalSemaphores = 0, VulkanSemaphore* SignalSemaphores = nullptr);

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
    RHI::RHIComputeContext* AcquireCommandContext() override;
    RHI::RHIComputeContext* ReleaseCommandContext(RHI::RHIComputeContext* Context) override;
    RHI::RHISyncPoint* Submit(RHI::RHICmdBuffer CmdBuffer) override;
    RHI::RHISyncPoint* Submit(const std::vector<RHI::RHICmdBuffer>& Cmds, const std::vector<RHI::RHISyncPoint*>& WaitPoints) override;
    void WaitIdle() override;

private:
    VulkanDevice* device_ = nullptr;
    VkQueue queue_ = VK_NULL_HANDLE;
    uint32_t familyIndex_ = UINT32_MAX;
    VulkanImageLayoutManager imageLayoutManager_;
    RHI::EQueueType QueueType_ = RHI::EQueueType::Graphics;
    std::vector<VulkanCommandContext*> AllContexts_;  // owns all pooled contexts
    std::vector<VulkanCommandContext*> FreeContexts_; // contexts available for acquire
    std::mutex ContextPoolMutex_;
};

// -------------------------------------------------------------------------------------------------
// Present executor backed by a VulkanQueue (graphics queue)
// -------------------------------------------------------------------------------------------------
class RHIVULKAN_API VulkanPresentExecutor final : public RHI::RHIPresentExecutor
{
public:
    explicit VulkanPresentExecutor(VulkanQueue* queue);
    void Present(RHI::RHISwapchain* Swapchain, RHI::RHISyncPoint* WaitSyncPoint = nullptr) override;
private:
    VulkanQueue* Queue = nullptr;
};

}