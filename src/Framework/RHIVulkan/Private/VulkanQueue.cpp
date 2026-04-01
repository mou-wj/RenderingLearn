#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandContex.h"
#include "VulkanResource.h"
#include "VulkanSwapchain.h"
#include "VulkanFuncWrapper.h"
#include <stdexcept>
#include <cassert>

namespace RHIVulkan{

VulkanQueue::VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex)
    : device_(device), queue_(queue), familyIndex_(familyIndex)
{
}

VulkanQueue::~VulkanQueue()
{
    for (VulkanCommandContext* ctx : AllContexts_)
    {
        delete ctx;
    }
    AllContexts_.clear();
    FreeContexts_.clear();
}

void VulkanQueue::SubmitCommandBuffer(VulkanCommandBuffer* CmdBuffer, uint32_t NumSignalSemaphores, VulkanSemaphore* SignalSemaphores)
{
    if (!CmdBuffer)
    {
        throw std::runtime_error("VulkanQueue::SubmitCommandBuffer: CmdBuffer is null");
    }

    VkCommandBuffer cmdBufferHandle = CmdBuffer->GetHandle();

    auto& waitFlags = CmdBuffer->WaitFlags;
    auto& waitSemaphores = CmdBuffer->WaitSemaphores;

    std::vector<VkSemaphore> waitSemaphoreHandles;
    std::vector<VkPipelineStageFlags> waitStageMasks;

    if (!waitSemaphores.empty())
    {
        size_t count = waitSemaphores.size();
        waitSemaphoreHandles.reserve(count);
        waitStageMasks.reserve(count);

        for (size_t i = 0; i < count; ++i)
        {
            VulkanSemaphore* semaphore = waitSemaphores[i];
            if (!semaphore)
            {
                continue;
            }

            waitSemaphoreHandles.push_back(semaphore->GetHandle());

            VkPipelineStageFlags stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            if (i < waitFlags.size())
            {
                stage = waitFlags[i];
            }
            waitStageMasks.push_back(stage);
        }
    }

    std::vector<VkSemaphore> signalSemaphoreHandles;
    if (NumSignalSemaphores > 0 && SignalSemaphores)
    {
        signalSemaphoreHandles.reserve(NumSignalSemaphores);
        for (uint32_t i = 0; i < NumSignalSemaphores; ++i)
        {
            signalSemaphoreHandles.push_back(SignalSemaphores[i].GetHandle());
        }
    }

    //添加commandbuffer的signal semaphore
    auto commandBufferSignalSemaphores = CmdBuffer->SignalSemaphores;
    for (auto& semaphore : commandBufferSignalSemaphores) {
        signalSemaphoreHandles.push_back(semaphore->GetHandle());
    }


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBufferHandle;

    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoreHandles.size());
    submitInfo.pWaitSemaphores = waitSemaphoreHandles.empty() ? nullptr : waitSemaphoreHandles.data();
    submitInfo.pWaitDstStageMask = waitStageMasks.empty() ? nullptr : waitStageMasks.data();

    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphoreHandles.size());
    submitInfo.pSignalSemaphores = signalSemaphoreHandles.empty() ? nullptr : signalSemaphoreHandles.data();

    if (!QueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE))
    {
        throw std::runtime_error("Failed to submit Vulkan command buffer");
    }

    device_->GetStagingManager()->GarbageCollect();
    CmdBuffer->GetImageLayoutManager()->TransferTo(imageLayoutManager_);
    CmdBuffer->GetImageLayoutManager()->Clear();
    imageLayoutManager_.PrintLayoutInfo();
}

void VulkanQueue::WaitIdle()
{
    if (!QueueWaitIdle(queue_))
    {
        throw std::runtime_error("Failed to wait for Vulkan queue to be idle!");
    }
}

void VulkanQueue::InitContextPool(RHI::EQueueType type, uint32_t poolSize)
{
    QueueType_ = type;
    std::lock_guard<std::mutex> lock(ContextPoolMutex_);
    for (uint32_t i = 0; i < poolSize; ++i)
    {
        VulkanCommandContext* ctx = new VulkanCommandContext(device_, this);
        AllContexts_.push_back(ctx);
        FreeContexts_.push_back(ctx);
    }
}

// ---- RHI::RHIQueue overrides ----

RHI::EQueueType VulkanQueue::GetType() const
{
    return QueueType_;
}

RHI::RHIComputeContext* VulkanQueue::AcquireCommandContext()
{
    std::lock_guard<std::mutex> lock(ContextPoolMutex_);
    if (!FreeContexts_.empty())
    {
        VulkanCommandContext* ctx = FreeContexts_.back();
        FreeContexts_.pop_back();
        return ctx;
    }
    // Pool exhausted — create a new context on demand
    VulkanCommandContext* ctx = new VulkanCommandContext(device_, this);
    AllContexts_.push_back(ctx);
    return ctx;
}

RHI::RHIComputeContext* VulkanQueue::ReleaseCommandContext(RHI::RHIComputeContext* Context)
{
    if (!Context)
    {
        return nullptr;
    }
    VulkanCommandContext* vulkanCtx = static_cast<VulkanCommandContext*>(Context);
    std::lock_guard<std::mutex> lock(ContextPoolMutex_);
    FreeContexts_.push_back(vulkanCtx);
    return nullptr;
}

RHI::RHISyncPoint* VulkanQueue::Submit(RHI::RHICmdBuffer CmdBuffer)
{
    VulkanCommandBuffer* commandBuffer = reinterpret_cast<VulkanCommandBuffer*>(CmdBuffer);
    if (!commandBuffer)
    {
        return nullptr;
    }

    SubmitCommandBuffer(commandBuffer);
    return new VulkanRHISyncPoint(QueueType_, commandBuffer->GetFence());
}

RHI::RHISyncPoint* VulkanQueue::Submit(const std::vector<RHI::RHICmdBuffer>& Cmds, const std::vector<RHI::RHISyncPoint*>& WaitPoints)
{
    (void)WaitPoints;

    RHI::RHISyncPoint* lastSyncPoint = nullptr;
    for (RHI::RHICmdBuffer cmdBuffer : Cmds)
    {
        lastSyncPoint = Submit(cmdBuffer);
    }

    return lastSyncPoint;
}

// -------------------------------------------------------------------------------------------------
// VulkanPresentExecutor
// -------------------------------------------------------------------------------------------------

VulkanPresentExecutor::VulkanPresentExecutor(VulkanQueue* queue)
    : Queue(queue)
{
}

void VulkanPresentExecutor::Present(RHI::RHISwapchain* Swapchain)
{
    auto* vulkanSwapchain = dynamic_cast<VulkanRHISwapchain*>(Swapchain);
    if (!vulkanSwapchain || !Queue)
    {
        return;
    }
    Queue->WaitIdle();
    vulkanSwapchain->GetSwapchain()->Present(Queue, nullptr);
}

// -------------------------------------------------------------------------------------------------
// VulkanRHISyncPoint
// -------------------------------------------------------------------------------------------------

VulkanRHISyncPoint::VulkanRHISyncPoint(RHI::EQueueType queueType, VulkanFence* inFence)
    : Fence(inFence)
{
    Type = queueType;
    Value = reinterpret_cast<uint64_t>(inFence);
}

bool VulkanRHISyncPoint::IsReached() const
{
    return Fence ? Fence->IsSignaled() : true;
}

void VulkanRHISyncPoint::Wait() const
{
    if (Fence)
    {
        Fence->Wait();
    }
}

}