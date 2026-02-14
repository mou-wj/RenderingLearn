#pragma once
#include "VUlkanDevice.h"
#include "VulkanResource.h"
#include <queue>
#include <mutex>

namespace RHIVulkan{

class VulkanDevice;
class VulkanEvent : public std::enable_shared_from_this<VulkanEvent> {
public:
    VulkanEvent(VulkanDevice* device);
    ~VulkanEvent();

    VkEvent GetHandle() const;

    void Set() const;
    void Reset() const;
    VkResult GetStatus() const;

private:
    VulkanDevice* device_;
    VkEvent event_;
};

class VulkanEventManager {
public:
    VulkanEventManager(VulkanDevice* device);
    ~VulkanEventManager();

    std::shared_ptr<VulkanEvent> Acquire();
    void Release(std::shared_ptr<VulkanEvent> evt);

private:
    VulkanDevice* device_;

    // 池：空闲可复用对象
    std::queue<std::shared_ptr<VulkanEvent>> pool_;

    // 管理所有创建的对象（用于析构和调试）
    std::vector<std::shared_ptr<VulkanEvent>> managedObjects_;

    std::mutex mutex_;
};



class VulkanSemaphore : public std::enable_shared_from_this<VulkanSemaphore> {
public:
    VulkanSemaphore(VulkanDevice* device);
    ~VulkanSemaphore();

    VkSemaphore GetHandle() const;

private:
    VulkanDevice* device_;
    VkSemaphore semaphore_;
};

class VulkanSemaphoreManager {
public:
    VulkanSemaphoreManager(VulkanDevice* device);
    ~VulkanSemaphoreManager();

    VulkanSemaphore* Acquire();
    void Release(VulkanSemaphore* sem);

private:
    VulkanDevice* device_;

    // 池：空闲对象
    std::queue<VulkanSemaphore*> pool_;

    // 管理所有创建对象
    std::vector<VulkanSemaphore*> managedObjects_;

    std::mutex mutex_;
};

}