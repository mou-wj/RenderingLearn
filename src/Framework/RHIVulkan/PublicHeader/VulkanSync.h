#pragma once
#include "VUlkanDevice.h"
#include "VulkanResource.h"
#include <queue>
#include <mutex>

namespace RHIVulkan{

class VulkanDevice;

class VulkanEvent {
public:
    VulkanEvent(VulkanDevice* device);
    ~VulkanEvent();

    VkEvent GetHandle() const;
    void Reset() const;
    void Set() const;
    VkResult GetStatus() const;

private:
    VulkanDevice* device_;
    VkEvent event_;
};


class VulkanSemaphore {
public:
    VulkanSemaphore(VulkanDevice* device);
    ~VulkanSemaphore();

    VkSemaphore GetHandle() const;

private:
    VulkanDevice* device_;
    VkSemaphore semaphore_;
};

class VulkanEventManager {
public:
    VulkanEventManager(VulkanDevice* device);
    ~VulkanEventManager();

    VkEvent Acquire();
    void Release(VkEvent evt);

private:
    VulkanDevice* device_;
    std::queue<VkEvent> pool_;
    std::mutex mutex_;
};


class VulkanSemaphoreManager {
public:
    VulkanSemaphoreManager(VulkanDevice* device);
    ~VulkanSemaphoreManager();

    VkSemaphore Acquire();
    void Release(VkSemaphore semaphore);

private:
    VulkanDevice* device_;
    std::queue<VkSemaphore> pool_;
    std::mutex mutex_;
};


}