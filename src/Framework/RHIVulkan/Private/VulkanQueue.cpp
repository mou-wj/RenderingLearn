#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandContex.h"
#include "VulkanResource.h"
#include "VulkanSwapchain.h"
#include "VulkanFuncWrapper.h"
#include <stdexcept>
#include <cassert>
#include <algorithm>

namespace RHIVulkan {

namespace {
VulkanCommandContext* CreateQueueContext(VulkanDevice* device, VulkanQueue* queue, RHI::EQueueType queueType)
{
    switch (queueType)
    {
    case RHI::EQueueType::Transfer:
        return new VulkanTransferContext(device, queue);
    case RHI::EQueueType::Compute:
        return new VulkanComputeContext(device, queue);
    case RHI::EQueueType::Graphics:
    default:
        return new VulkanGraphicContext(device, queue);
    }
}
}

// -------------------------------------------------------------------------------------------------
// VulkanQueue
// -------------------------------------------------------------------------------------------------

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

    auto commandBufferSignalSemaphores = CmdBuffer->SignalSemaphores;
    for (auto& semaphore : commandBufferSignalSemaphores)
    {
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

    if (device_->GetSyncPointManager())
    {
        device_->GetSyncPointManager()->GarbageCollect();
    }

    VkFence submitFence = CmdBuffer->GetFence() ? CmdBuffer->GetFence()->GetHandle() : VK_NULL_HANDLE;
    if (!QueueSubmit(queue_, 1, &submitInfo, submitFence))
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

    if (device_ && device_->GetSyncPointManager())
    {
        device_->GetSyncPointManager()->GarbageCollect();
    }
}

void VulkanQueue::InitContextPool(RHI::EQueueType type, uint32_t poolSize)
{
    QueueType_ = type;
    std::lock_guard<std::mutex> lock(ContextPoolMutex_);
    for (uint32_t i = 0; i < poolSize; ++i)
    {
        VulkanCommandContext* ctx = CreateQueueContext(device_, this, QueueType_);
        AllContexts_.push_back(ctx);
        FreeContexts_.push_back(ctx);
    }
}

RHI::EQueueType VulkanQueue::GetType() const
{
    return QueueType_;
}

RHI::RHIContextBase* VulkanQueue::AcquireCommandContext()
{
    std::lock_guard<std::mutex> lock(ContextPoolMutex_);
    if (!FreeContexts_.empty())
    {
        VulkanCommandContext* ctx = FreeContexts_.back();
        FreeContexts_.pop_back();
        return ctx;
    }

    VulkanCommandContext* ctx = CreateQueueContext(device_, this, QueueType_);
    AllContexts_.push_back(ctx);
    return ctx;
}

RHI::RHIContextBase* VulkanQueue::ReleaseCommandContext(RHI::RHIContextBase* Context)
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
    auto* syncPointManager = device_ ? device_->GetSyncPointManager() : nullptr;
    return syncPointManager ? syncPointManager->Acquire(QueueType_, commandBuffer->GetFence()) : nullptr;
}

RHI::RHISyncPoint* VulkanQueue::Submit(const std::vector<RHI::RHICmdBuffer>& Cmds, const std::vector<RHI::RHISyncPoint*>& WaitPoints)
{
    for (RHI::RHISyncPoint* waitPoint : WaitPoints)
    {
        if (waitPoint && !waitPoint->IsReached())
        {
            waitPoint->Wait();
        }
    }

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

void VulkanPresentExecutor::Present(RHI::RHISwapchain* Swapchain, RHI::RHISyncPoint* WaitSyncPoint)
{
    auto* vulkanSwapchain = dynamic_cast<VulkanRHISwapchain*>(Swapchain);
    if (!vulkanSwapchain || !Queue)
    {
        return;
    }

    if (WaitSyncPoint)
    {
        WaitSyncPoint->Wait();
    }
    else
    {
        Queue->WaitIdle();
    }

    vulkanSwapchain->GetSwapchain()->Present(Queue, nullptr);
}
}