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

VkPipelineStageFlags ToVkPipelineStage(RHI::ERHIPipelineStage stage)
{
    switch (stage)
    {
    case RHI::ERHIPipelineStage::TopOfPipe:
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case RHI::ERHIPipelineStage::DrawIndirect:
        return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    case RHI::ERHIPipelineStage::VertexInput:
        return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case RHI::ERHIPipelineStage::VertexShader:
        return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    case RHI::ERHIPipelineStage::FragmentShader:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case RHI::ERHIPipelineStage::EarlyFragmentTests:
        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    case RHI::ERHIPipelineStage::LateFragmentTests:
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case RHI::ERHIPipelineStage::ColorAttachmentOutput:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case RHI::ERHIPipelineStage::ComputeShader:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case RHI::ERHIPipelineStage::Transfer:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case RHI::ERHIPipelineStage::BottomOfPipe:
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case RHI::ERHIPipelineStage::AllCommands:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    case RHI::ERHIPipelineStage::None:
    default:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
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
    if (!VKFunc::QueueSubmit(queue_, 1, &submitInfo, submitFence))
    {
        throw std::runtime_error("Failed to submit Vulkan command buffer");
    }

    device_->GetStagingManager()->GarbageCollect();
    CmdBuffer->GetImageLayoutManager()->TransferTo(imageLayoutManager_);
    CmdBuffer->GetImageLayoutManager()->Clear();
    CmdBuffer->WaitFlags.clear();
    CmdBuffer->WaitSemaphores.clear();
    CmdBuffer->SubmittedWaitSemaphores.clear();
    CmdBuffer->SignalSemaphores.clear();
    imageLayoutManager_.PrintLayoutInfo();
}

void VulkanQueue::SubmitSignalSemaphore(VulkanSemaphore* SignalSemaphore)
{
    if (!SignalSemaphore)
    {
        return;
    }

    VkSemaphore signalSemaphoreHandle = SignalSemaphore->GetHandle();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 0;
    submitInfo.pCommandBuffers = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphoreHandle;

    if (device_->GetSyncPointManager())
    {
        device_->GetSyncPointManager()->GarbageCollect();
    }

    if (!VKFunc::QueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE))
    {
        throw std::runtime_error("Failed to submit Vulkan signal semaphore");
    }
}

void VulkanQueue::WaitIdle()
{
    if (!VKFunc::QueueWaitIdle(queue_))
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

    VulkanCommandContext* vulkanCtx = dynamic_cast<VulkanCommandContext*>(Context);
    std::lock_guard<std::mutex> lock(ContextPoolMutex_);
    FreeContexts_.push_back(vulkanCtx);
    return nullptr;
}

RHI::RHISubmitResult VulkanQueue::Submit(RHI::RHICmdBuffer CmdBuffer)
{
    RHI::RHISubmitResult result{};
    VulkanCommandBuffer* commandBuffer = reinterpret_cast<VulkanCommandBuffer*>(CmdBuffer);
    if (!commandBuffer)
    {
        return result;
    }

    VulkanSemaphore* signalSemaphore = device_ ? device_->GetSemaphoreManager()->Acquire(true) : nullptr;
    SubmitCommandBuffer(commandBuffer, signalSemaphore ? 1 : 0, signalSemaphore);
    auto* syncPointManager = device_ ? device_->GetSyncPointManager() : nullptr;
    if (!syncPointManager)
    {
        return result;
    }

    result.Completion = syncPointManager->AcquireCompletion(QueueType_, commandBuffer->GetFence());
    result.Dependency = syncPointManager->AcquireDependency(QueueType_, signalSemaphore, signalSemaphore != nullptr);
    return result;
}

RHI::RHISubmitResult VulkanQueue::Submit(const std::vector<RHI::RHICmdBuffer>& Cmds, const std::vector<RHI::RHIWaitInfo>& WaitInfos)
{
    RHI::RHISubmitResult result{};
    std::vector<VulkanSemaphore*> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<RHI::RHISyncDependency*> consumedWaitDependencies;
    for (const RHI::RHIWaitInfo& waitInfo : WaitInfos)
    {
        if (!waitInfo.Dependency)
        {
            continue;
        }

        if (auto* vulkanDependency = dynamic_cast<VulkanRHISyncDependency*>(waitInfo.Dependency))
        {
            if (auto* semaphore = vulkanDependency->GetSemaphore())
            {
                waitSemaphores.push_back(semaphore);
                waitStages.push_back(ToVkPipelineStage(waitInfo.WaitStage));
                consumedWaitDependencies.push_back(waitInfo.Dependency);
                continue;
            }
        }
    }

    RHI::RHISubmitResult lastResult{};
    for (size_t cmdIndex = 0; cmdIndex < Cmds.size(); ++cmdIndex)
    {
        RHI::RHICmdBuffer cmdBuffer = Cmds[cmdIndex];
        if (cmdIndex == 0 && !waitSemaphores.empty())
        {
            auto* commandBuffer = reinterpret_cast<VulkanCommandBuffer*>(cmdBuffer);
            if (commandBuffer)
            {
                for (size_t waitIndex = 0; waitIndex < waitSemaphores.size(); ++waitIndex)
                {
                    commandBuffer->AddWaitSemaphores(waitStages[waitIndex], { waitSemaphores[waitIndex] });
                }
            }
        }
        lastResult = Submit(cmdBuffer);
    }

    if (auto* syncPointManager = device_ ? device_->GetSyncPointManager() : nullptr)
    {
        for (RHI::RHISyncDependency* dependency : consumedWaitDependencies)
        {
            syncPointManager->TryRecycle(dependency);
        }
    }

    result = lastResult;
    return result;
}

// -------------------------------------------------------------------------------------------------
// VulkanPresentExecutor
// -------------------------------------------------------------------------------------------------

VulkanPresentExecutor::VulkanPresentExecutor(VulkanQueue* queue)
    : Queue(queue)
{
}

void VulkanPresentExecutor::Present(RHI::RHISwapchain* Swapchain, RHI::RHISyncDependency* WaitDependency)
{
    auto* vulkanSwapchain = dynamic_cast<VulkanRHISwapchain*>(Swapchain);
    if (!vulkanSwapchain || !Queue)
    {
        return;
    }

    vulkanSwapchain->Present(Queue, WaitDependency);
}
}