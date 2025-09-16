#include "VulkanQueue.h"
#include "VulkanDevice.h" // 如果需要使用 VulkanDevice 中的接口

#include <stdexcept>

namespace RHIVulkan{

VulkanQueue::VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex)
    : device_(device), queue_(queue), familyIndex_(familyIndex)
{
}

void VulkanQueue::Submit(const std::vector<VkSubmitInfo>& submitInfos, VkFence fence) const
{
    if (vkQueueSubmit(queue_, static_cast<uint32_t>(submitInfos.size()), submitInfos.data(), fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit to Vulkan queue!");
    }
}

void VulkanQueue::WaitIdle() const
{
    if (vkQueueWaitIdle(queue_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to wait for Vulkan queue to be idle!");
    }
}

}