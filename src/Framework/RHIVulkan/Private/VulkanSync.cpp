#include "VulkanSync.h"
#include <stdexcept>

namespace RHIVulkan{

// ================= VulkanEvent =================
VulkanEvent::VulkanEvent(VulkanDevice* device)
    : device_(device)
{
    VkEventCreateInfo info{ VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
    if (vkCreateEvent(device_->GetDevice(), &info, nullptr, &event_) != VK_SUCCESS)
        throw std::runtime_error("Failed to create VkEvent");
}

VulkanEvent::~VulkanEvent()
{
    if (event_)
        vkDestroyEvent(device_->GetDevice(), event_, nullptr);
}

void VulkanEvent::Reset() const
{
    vkResetEvent(device_->GetDevice(), event_);
}

void VulkanEvent::Set() const
{
    vkSetEvent(device_->GetDevice(), event_);
}

VkResult VulkanEvent::GetStatus() const
{
    return vkGetEventStatus(device_->GetDevice(), event_);
}


// ================= VulkanEventManager =================
VulkanEventManager::VulkanEventManager(VulkanDevice* device)
    : device_(device)
{
}

VulkanEventManager::~VulkanEventManager()
{
    while (!pool_.empty())
    {
        vkDestroyEvent(device_->GetDevice(), pool_.front(), nullptr);
        pool_.pop();
    }
}

VkEvent VulkanEventManager::Acquire()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!pool_.empty())
    {
        VkEvent evt = pool_.front();
        pool_.pop();
        return evt;
    }

    VkEventCreateInfo info{ VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
    VkEvent evt;
    if (vkCreateEvent(device_->GetDevice(), &info, nullptr, &evt) != VK_SUCCESS)
        throw std::runtime_error("Failed to create VkEvent");

    return evt;
}

void VulkanEventManager::Release(VkEvent evt)
{
    std::lock_guard<std::mutex> lock(mutex_);
    vkResetEvent(device_->GetDevice(), evt);
    pool_.push(evt);
}


// --- VulkanSemaphore ---
VulkanSemaphore::VulkanSemaphore(VulkanDevice* device)
    : device_(device) {
    VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    if (vkCreateSemaphore(device_->GetDevice(), &info, nullptr, &semaphore_) != VK_SUCCESS)
        throw std::runtime_error("Failed to create VkSemaphore");
}

VulkanSemaphore::~VulkanSemaphore() {
    if (semaphore_)
        vkDestroySemaphore(device_->GetDevice(), semaphore_, nullptr);
}

VkSemaphore VulkanSemaphore::GetHandle() const {
    return semaphore_;
}

// --- VulkanSemaphoreManager ---
VulkanSemaphoreManager::VulkanSemaphoreManager(VulkanDevice* device)
    : device_(device) {
}

VulkanSemaphoreManager::~VulkanSemaphoreManager() {
    while (!pool_.empty()) {
        vkDestroySemaphore(device_->GetDevice(), pool_.front(), nullptr);
        pool_.pop();
    }
}

VkSemaphore VulkanSemaphoreManager::Acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!pool_.empty()) {
        VkSemaphore sem = pool_.front();
        pool_.pop();
        return sem;
    }

    VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore sem;
    if (vkCreateSemaphore(device_->GetDevice(), &info, nullptr, &sem) != VK_SUCCESS)
        throw std::runtime_error("Failed to create VkSemaphore");
    return sem;
}

void VulkanSemaphoreManager::Release(VkSemaphore semaphore) {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.push(semaphore);
}

}
