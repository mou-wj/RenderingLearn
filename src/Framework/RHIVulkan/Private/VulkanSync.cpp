#include "VulkanSync.h"
#include "VulkanFuncWrapper.h"
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
        managedObjects_ = {}; // shared_ptrԶ
    }

    std::shared_ptr<VulkanEvent> VulkanEventManager::Acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto evt = pool_.front();
            pool_.pop();
            evt->Reset();
            return evt;
        }

        //ؿʱ¶󣬲 managedObjects
        auto evt = std::make_shared<VulkanEvent>(device_);
        managedObjects_.push_back(evt);
        return evt;
    }

    void VulkanEventManager::Release(std::shared_ptr<VulkanEvent> evt) {
        if (!evt) return;
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(evt);
    }


    VulkanSemaphore::VulkanSemaphore(VulkanDevice* device, bool isBinary, uint64_t initialValue)
        : device_(device),isBinary_(isBinary), value_(initialValue)
    {
        VkSemaphoreTypeCreateInfo typeCreateInfo{};
        typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        if (!isBinary) {
            typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            typeCreateInfo.initialValue = initialValue;
            value_ = initialValue;
        }
        else {
            typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
            typeCreateInfo.initialValue = 0;
            value_ = 0;
        }
        
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = &typeCreateInfo;
        createInfo.flags = 0;
        VKFunc::CreateSemaphore_(device_->GetHandle(), &createInfo, &semaphore_);
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        if (semaphore_ != VK_NULL_HANDLE) {
            VKFunc::DestroySemaphore(device_->GetHandle(), semaphore_);
            semaphore_ = VK_NULL_HANDLE;
        }
    }

    VkSemaphore VulkanSemaphore::GetHandle() const { return semaphore_; }



    uint64_t VulkanSemaphore::GetCurrentValue() {
        uint64_t value = 0;
        VkResult res = vkGetSemaphoreCounterValue(device_->GetHandle(), semaphore_, &value);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to get timeline semaphore value");
        }
        return value;
    }

    void VulkanSemaphore::Wait(uint64_t Value, uint64_t TimeoutNS) {
        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.flags = 0;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &semaphore_;
        waitInfo.pValues = &Value;
        VkResult res = vkWaitSemaphores(device_->GetHandle(), &waitInfo, TimeoutNS);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to wait for timeline semaphore");
        }
        value_ = Value;
    }

    VulkanSemaphoreManager::VulkanSemaphoreManager(VulkanDevice* device)
        : device_(device) {
    }

    VulkanSemaphoreManager::~VulkanSemaphoreManager() {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_ = {};
        for (auto sem : managedObjects_) {
            delete sem; // ж
        }
        managedObjects_.clear();
    }

    VulkanSemaphore* VulkanSemaphoreManager::Acquire(bool requireUnsignaled) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (requireUnsignaled)
        {
            VulkanSemaphore* sem = new VulkanSemaphore(device_);
            managedObjects_.push_back(sem);
            return sem;
        }

        if (!pool_.empty())
        {
            VulkanSemaphore* sem = pool_.front();
            pool_.pop();
            return sem;
        }

        VulkanSemaphore* sem = new VulkanSemaphore(device_);
        managedObjects_.push_back(sem);
        return sem;
    }

    void VulkanSemaphoreManager::Release(VulkanSemaphore* sem) {
        if (!sem) return;
        std::lock_guard<std::mutex> lock(mutex_);

        if (pool_.size() >= MaxSemaphorePoolSize)
        {
            auto it = std::find(managedObjects_.begin(), managedObjects_.end(), sem);
            if (it != managedObjects_.end())
            {
                managedObjects_.erase(it);
            }
            delete sem;
        }
        else
        {
            pool_.push(sem);
        }
    }

// -------------------------------------------------------------------------------------------------
// VulkanFence Implementation
// -------------------------------------------------------------------------------------------------

VulkanFence::VulkanFence(VulkanDevice* device)
{
    Device = device;
    VkDevice vkDevice = Device->GetHandle();
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    if (!VKFunc::CreateFence(vkDevice, &fenceInfo, &Fence)) {
        // Handle error
    }
}

VulkanFence::~VulkanFence()
{
    if (Fence) {
        VKFunc::DestroyFence(Device->GetHandle(), Fence);
        Fence = VK_NULL_HANDLE;
    }
}

bool VulkanFence::IsSignaled() const
{
    if (!Fence || !Device) return false;
    VkResult result = vkGetFenceStatus(Device->GetHandle(), Fence); // 若有VKFunc包装可替换
    return result == VK_SUCCESS;
}

void VulkanFence::Reset()
{
    if (!Fence || !Device) return;
    VKFunc::ResetFences(Device->GetHandle(), 1, &Fence);
}

void VulkanFence::Wait()
{
    if (!Fence || !Device) return;
    VKFunc::WaitForFences(Device->GetHandle(), 1, &Fence, VK_TRUE, UINT64_MAX);
}

// -------------------------------------------------------------------------------------------------
// VulkanFenceManager Implementation
// -------------------------------------------------------------------------------------------------

VulkanFenceManager::VulkanFenceManager(VulkanDevice* deviceIn)
    : device(deviceIn)
{
}

VulkanFenceManager::~VulkanFenceManager()
{
    allFences.clear();
    availableFences.clear();
    pendingFences.clear();
}

VulkanFence* VulkanFenceManager::AcquireFence()
{
    if (!availableFences.empty())
    {
        VulkanFence* fence = availableFences.front();
        availableFences.pop_front();
        return fence;
    }

    auto newFence = std::make_unique<VulkanFence>(device);
    VulkanFence* ptr = newFence.get();
    allFences.push_back(std::move(newFence));
    return ptr;
}

void VulkanFenceManager::ReleaseFence(VulkanFence* fence)
{
    if (fence)
    {
        pendingFences.push_back(fence);
    }
}

void VulkanFenceManager::GarbageCollect()
{
    size_t count = pendingFences.size();
    for (size_t i = 0; i < count; ++i)
    {
        VulkanFence* fence = pendingFences.front();
        pendingFences.pop_front();

        if (fence->IsSignaled())
        {
            availableFences.push_back(fence);
        }
        else
        {
            pendingFences.push_back(fence);
        }
    }
}

// -------------------------------------------------------------------------------------------------
// VulkanRHISyncPoint (Timeline Semaphore) Implementation
// -------------------------------------------------------------------------------------------------

VulkanRHISyncPoint::VulkanRHISyncPoint(VulkanDevice* device, RHI::EQueueType queueType, uint64_t initialValue, bool isBinary)
	: device_(device)
{
    semaphore_ = new VulkanSemaphore(device_, isBinary,initialValue);
    Type = queueType;
}

VulkanRHISyncPoint::~VulkanRHISyncPoint() {
    delete semaphore_;
}

uint64_t VulkanRHISyncPoint::GetCurrentValue() {
    return semaphore_->GetCurrentValue();
}

void VulkanRHISyncPoint::Wait(uint64_t Value, uint64_t TimeoutNS) {
    semaphore_->Wait(Value, TimeoutNS);
}

}
