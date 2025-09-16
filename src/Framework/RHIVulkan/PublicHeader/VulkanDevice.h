#pragma once

#include <vulkan/vulkan.h>
#include "VulkanRHIApi.h"


#include <vector>
#include <string>
#include <memory>
namespace RHIVulkan{

class VulkanMemoryManager; // 前向声明
class VulkanQueue; // 前向声明
class VulkanDescriptorPoolManager; // 前向声明
class VulkanSemaphoreManager; // 前向声明
class VulkanEventManager; // 前向声明

class VulkanDevice
{
public:
    VulkanDevice(VulkanRHIApi* rhiApi,VkPhysicalDevice physicalDevice);

    ~VulkanDevice();
    bool Init(const std::vector<const char*>& enabledLayers,const std::vector<const char*>& enabledExtensions);
    void Destroy();
    VulkanRHIApi* GetRhiApi() const { return rhiApi_; }
    VkDevice GetDevice() const { return device_; }
    VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice_; }
    VulkanQueue* GetGraphicsQueue() const { return graphicsQueue_; }
    VulkanQueue* GetComputeQueue() const { return computeQueue_; }
    VulkanQueue* GetTransferQueue() const { return transferQueue_; }
    VulkanQueue* GetPresentQueue() const { return presentQueue_; }

    uint32_t GetGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex_; }
    uint32_t GetComputeQueueFamilyIndex() const { return computeQueueFamilyIndex_; }
    uint32_t GetTransferQueueFamilyIndex() const { return transferQueueFamilyIndex_; }
    uint32_t GetPresentQueueFamilyIndex() const { return presentQueueFamilyIndex_; }

    VulkanMemoryManager* GetMemoryManager() const { return memoryManager_; }
    VulkanDescriptorPoolManager* GetDescriptorPoolManager() const { return descriptorPoolManager_; }
    VkInstance GetInstance() const { return rhiApi_->GetInstance(); }
    bool InitPresentQueue(VkSurfaceKHR Surface);
    VulkanSemaphoreManager* GetSemaphoreManager() const { return semaphoreManager_; }
	VulkanEventManager* GetEventManager() const { return eventManager_; }

private:
    void SelectQueueFamilies(VkPhysicalDevice physicalDevice);
    void CreateLogicalDevice(VkPhysicalDevice physicalDevice,
                             const std::vector<const char*>& extensions,
                             const std::vector<const char*>& layers);

    VulkanRHIApi *rhiApi_;

    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_{VK_NULL_HANDLE};

    uint32_t graphicsQueueFamilyIndex_ = UINT32_MAX;
    uint32_t computeQueueFamilyIndex_ = UINT32_MAX;
    uint32_t transferQueueFamilyIndex_ = UINT32_MAX;
    uint32_t presentQueueFamilyIndex_ = UINT32_MAX;

    VulkanQueue *graphicsQueue_;
    VulkanQueue *computeQueue_;
    VulkanQueue *transferQueue_;
    VulkanQueue *presentQueue_;

    VulkanMemoryManager* memoryManager_;
    VulkanDescriptorPoolManager* descriptorPoolManager_ = nullptr;
	VulkanSemaphoreManager* semaphoreManager_ = nullptr;
	VulkanEventManager* eventManager_ = nullptr;

};

}