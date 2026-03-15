#include "VulkanQueue.h"
#include "VulkanDevice.h" // 如果需要使用 VulkanDevice 中的接口
#include "VulkanCommandBuffer.h"
#include "VulkanFuncWrapper.h"
#include <stdexcept>

namespace RHIVulkan{

VulkanQueue::VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex)
    : device_(device), queue_(queue), familyIndex_(familyIndex)
{
}

void VulkanQueue::Submit(VulkanCommandBuffer* CmdBuffer, uint32_t NumSignalSemaphores, VulkanSemaphore* SignalSemaphores)
{
    if (!CmdBuffer)
    {
        throw std::runtime_error("VulkanQueue::Submit: CmdBuffer is null");
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
}

void VulkanQueue::WaitIdle() const
{
    if (!QueueWaitIdle(queue_))
    {
        throw std::runtime_error("Failed to wait for Vulkan queue to be idle!");
    }
}

}