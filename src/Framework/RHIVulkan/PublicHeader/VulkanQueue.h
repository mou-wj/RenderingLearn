#pragma once

#include "VulkanDevice.h"
#include <vector>

namespace RHIVulkan{

// 前向声明以避免循环依赖
class VulkanDevice;

class VulkanQueue
{
public:
    VulkanQueue() = default;
    VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex);

    void Submit(const std::vector<VkSubmitInfo>& submitInfos, VkFence fence = VK_NULL_HANDLE) const;
    void WaitIdle() const;

    VkQueue GetHandle() const { return queue_; }
    uint32_t GetFamilyIndex() const { return familyIndex_; }
    VulkanDevice* GetDevice() const { return device_; }

private:
    VulkanDevice* device_ = nullptr;
    VkQueue queue_ = VK_NULL_HANDLE;
    uint32_t familyIndex_ = UINT32_MAX;
};

}