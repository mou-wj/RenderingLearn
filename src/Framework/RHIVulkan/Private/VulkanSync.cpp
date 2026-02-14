#include "VulkanSync.h"
#include <stdexcept>

namespace RHIVulkan{

    VulkanEvent::VulkanEvent(VulkanDevice* device)
        : device_(device)
    {
        VkEventCreateInfo info{ VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
        info.flags = 0;
        vkCreateEvent(device_->GetHandle(), &info, nullptr, &event_);
    }

    VulkanEvent::~VulkanEvent()
    {
        if (event_ != VK_NULL_HANDLE) {
            vkDestroyEvent(device_->GetHandle(), event_, nullptr);
            event_ = VK_NULL_HANDLE;
        }
    }

    VkEvent VulkanEvent::GetHandle() const { return event_; }

    void VulkanEvent::Set() const { vkSetEvent(device_->GetHandle(), event_); }
    void VulkanEvent::Reset() const { vkResetEvent(device_->GetHandle(), event_); }
    VkResult VulkanEvent::GetStatus() const { return vkGetEventStatus(device_->GetHandle(), event_); }

    VulkanEventManager::VulkanEventManager(VulkanDevice* device)
        : device_(device) {
    }

    VulkanEventManager::~VulkanEventManager() {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_ = {};
        managedObjects_ = {}; // 所有 shared_ptr 自动析构
    }

    std::shared_ptr<VulkanEvent> VulkanEventManager::Acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto evt = pool_.front();
            pool_.pop();
            evt->Reset();
            return evt;
        }

        // 池空时创建新对象，并加入 managedObjects
        auto evt = std::make_shared<VulkanEvent>(device_);
        managedObjects_.push_back(evt);
        return evt;
    }

    void VulkanEventManager::Release(std::shared_ptr<VulkanEvent> evt) {
        if (!evt) return;
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(evt);
    }


    VulkanSemaphore::VulkanSemaphore(VulkanDevice* device)
        : device_(device)
    {
        VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        info.flags = 0;
        vkCreateSemaphore(device_->GetHandle(), &info, nullptr, &semaphore_);
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        if (semaphore_ != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_->GetHandle(), semaphore_, nullptr);
            semaphore_ = VK_NULL_HANDLE;
        }
    }

    VkSemaphore VulkanSemaphore::GetHandle() const { return semaphore_; }


    VulkanSemaphoreManager::VulkanSemaphoreManager(VulkanDevice* device)
        : device_(device) {
    }

    VulkanSemaphoreManager::~VulkanSemaphoreManager() {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_ = {};
        for (auto sem : managedObjects_) {
            delete sem; // 所有对象析构
        }
        managedObjects_.clear();
    }

    VulkanSemaphore* VulkanSemaphoreManager::Acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            VulkanSemaphore* sem = pool_.front();
            pool_.pop();
            return sem;
        }

        // 池空时创建新对象并加入管理列表
        VulkanSemaphore* sem = new VulkanSemaphore(device_);
        managedObjects_.push_back(sem);
        return sem;
    }

    void VulkanSemaphoreManager::Release(VulkanSemaphore* sem) {
        if (!sem) return;
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(sem);
    }


}
