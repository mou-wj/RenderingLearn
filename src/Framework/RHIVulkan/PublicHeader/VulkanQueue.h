#pragma once

#include "VulkanDevice.h"
#include "VulkanBarriers.h"
#include <vector>

namespace RHIVulkan{

// 前向声明以避免循环依赖
class VulkanDevice;
class VulkanCommandBuffer;
class VulkanSemaphore;

class VulkanQueue
{
public:
    VulkanQueue() = default;
    VulkanQueue(VulkanDevice* device, VkQueue queue, uint32_t familyIndex);

    void Submit(VulkanCommandBuffer* CmdBuffer, uint32_t NumSignalSemaphores = 0, VulkanSemaphore* SignalSemaphores = nullptr);
    void WaitIdle() const;

    VkQueue GetHandle() const { return queue_; }
    uint32_t GetFamilyIndex() const { return familyIndex_; }
    VulkanDevice* GetDevice() const { return device_; }
	VulkanImageLayoutManager& GetImageLayoutManager() { return imageLayoutManager_; }
private:
    VulkanDevice* device_ = nullptr;
    VkQueue queue_ = VK_NULL_HANDLE;
    uint32_t familyIndex_ = UINT32_MAX;
    VulkanImageLayoutManager imageLayoutManager_;
};

}