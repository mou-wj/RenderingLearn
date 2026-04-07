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
        managedObjects_ = {}; // ���� shared_ptr �Զ�����
    }

    std::shared_ptr<VulkanEvent> VulkanEventManager::Acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto evt = pool_.front();
            pool_.pop();
            evt->Reset();
            return evt;
        }

        // �ؿ�ʱ�����¶��󣬲����� managedObjects
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
        VKFunc::CreateSemaphore_(device_->GetHandle(), &info, &semaphore_);
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        if (semaphore_ != VK_NULL_HANDLE) {
            VKFunc::DestroySemaphore(device_->GetHandle(), semaphore_);
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
            delete sem; // ���ж�������
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
// VulkanRHISyncPointManager Implementation
// -------------------------------------------------------------------------------------------------

VulkanRHISyncPointManager::VulkanRHISyncPointManager(VulkanDevice* device)
    : Device(device)
{
}

VulkanRHISyncPointManager::~VulkanRHISyncPointManager()
{
    WaitAndRecycleAll();
    std::lock_guard<std::mutex> lock(Mutex);
    PendingSyncPoints.clear();
    FreeSyncPoints.clear();
    AllSyncPoints.clear();
    PendingDependencies.clear();
    FreeDependencies.clear();
    AllDependencies.clear();
}

RHI::RHISyncPoint* VulkanRHISyncPointManager::AcquireCompletion(
    RHI::EQueueType queueType,
    VulkanFence* fence)
{
    std::lock_guard<std::mutex> lock(Mutex);
    GarbageCollect_NoLock();

    RHI::RHISyncPoint* syncPoint = nullptr;
    if (!FreeSyncPoints.empty())
    {
        syncPoint = FreeSyncPoints.back();
        FreeSyncPoints.pop_back();
    }
    else
    {
        auto newSyncPoint = std::make_unique<VulkanRHISyncPoint>();
        syncPoint = newSyncPoint.get();
        AllSyncPoints.push_back(std::move(newSyncPoint));
    }

    if (auto vulkanSyncPoint = dynamic_cast<VulkanRHISyncPoint*>(syncPoint))
    {
        vulkanSyncPoint->Activate(queueType, fence, NextValue++, this);
    }
    PendingSyncPoints.push_back(syncPoint);
    return syncPoint;
}

RHI::RHISyncDependency* VulkanRHISyncPointManager::AcquireDependency(
    RHI::EQueueType queueType,
    VulkanSemaphore* semaphore,
    bool ownsSemaphore)
{
    if (!semaphore)
    {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(Mutex);
    GarbageCollect_NoLock();

    RHI::RHISyncDependency* dependency = nullptr;
    if (!FreeDependencies.empty())
    {
        dependency = FreeDependencies.back();
        FreeDependencies.pop_back();
    }
    else
    {
        auto newDependency = std::make_unique<VulkanRHISyncDependency>();
        dependency = newDependency.get();
        AllDependencies.push_back(std::move(newDependency));
    }

    if (auto vulkanDependency = dynamic_cast<VulkanRHISyncDependency*>(dependency))
    {
        vulkanDependency->Activate(queueType, semaphore, ownsSemaphore, NextValue++, this);
    }
    PendingDependencies.push_back(dependency);
    return dependency;
}

void VulkanRHISyncPointManager::GarbageCollect()
{
    std::lock_guard<std::mutex> lock(Mutex);
    GarbageCollect_NoLock();
}

void VulkanRHISyncPointManager::TryRecycle(RHI::RHISyncPoint* syncPoint)
{
    if (!syncPoint)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(Mutex);
    if (auto vulkanSyncPoint = dynamic_cast<VulkanRHISyncPoint*>(syncPoint))
    {
        if (!vulkanSyncPoint->bPending)
        {
            return;
        }

        const bool reachedByFence = vulkanSyncPoint->Fence && vulkanSyncPoint->Fence->IsSignaled();
        if (reachedByFence)
        {
            Recycle_NoLock(syncPoint);
        }
    }
}

void VulkanRHISyncPointManager::TryRecycle(RHI::RHISyncDependency* dependency)
{
    if (!dependency)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(Mutex);
    if (auto* vulkanDependency = dynamic_cast<VulkanRHISyncDependency*>(dependency))
    {
        if (!vulkanDependency->bPending)
        {
            return;
        }

        Recycle_NoLock(dependency);
    }
}

void VulkanRHISyncPointManager::WaitAndRecycleAll()
{
    std::lock_guard<std::mutex> lock(Mutex);
    for (RHI::RHISyncPoint* syncPoint : PendingSyncPoints)
    {
        if (auto vulkanSyncPoint = dynamic_cast<VulkanRHISyncPoint*>(syncPoint))
        {
            if (vulkanSyncPoint && vulkanSyncPoint->Fence)
            {
                vulkanSyncPoint->Fence->Wait();
            }
            if (vulkanSyncPoint)
            {
                vulkanSyncPoint->bPending = false;
                vulkanSyncPoint->Fence = nullptr;
                vulkanSyncPoint->Owner = this;
                FreeSyncPoints.push_back(syncPoint);
            }
        }
    }
    PendingSyncPoints.clear();

    for (RHI::RHISyncDependency* dependency : PendingDependencies)
    {
        if (auto* vulkanDependency = dynamic_cast<VulkanRHISyncDependency*>(dependency))
        {
            if (vulkanDependency->Semaphore && vulkanDependency->bOwnsSemaphore)
            {
                if (auto* semaphoreManager = Device ? Device->GetSemaphoreManager() : nullptr)
                {
                    semaphoreManager->Release(vulkanDependency->Semaphore);
                }
            }
            vulkanDependency->bPending = false;
            vulkanDependency->Semaphore = nullptr;
            vulkanDependency->bOwnsSemaphore = false;
            vulkanDependency->Owner = this;
            FreeDependencies.push_back(dependency);
        }
    }
    PendingDependencies.clear();
}

void VulkanRHISyncPointManager::GarbageCollect_NoLock()
{
    for (auto it = PendingSyncPoints.begin(); it != PendingSyncPoints.end();)
    {
        RHI::RHISyncPoint* syncPoint = *it;
        bool reached = false;
        if (auto vulkanSyncPoint = dynamic_cast<VulkanRHISyncPoint*>(syncPoint))
        {
            reached = !vulkanSyncPoint || (vulkanSyncPoint->Fence && vulkanSyncPoint->Fence->IsSignaled());
        }
        
        if (reached)
        {
            if (syncPoint)
            {
                if (auto vulkanSyncPoint = dynamic_cast<VulkanRHISyncPoint*>(syncPoint))
                {
                    vulkanSyncPoint->bPending = false;
                    vulkanSyncPoint->Fence = nullptr;
                    vulkanSyncPoint->Owner = this;
                }
                FreeSyncPoints.push_back(syncPoint);
            }
            it = PendingSyncPoints.erase(it);
            continue;
        }
        ++it;
    }

    for (auto it = PendingDependencies.begin(); it != PendingDependencies.end();)
    {
        auto* dependency = *it;
        if (!dependency)
        {
            it = PendingDependencies.erase(it);
            continue;
        }

        if (auto* vulkanDependency = dynamic_cast<VulkanRHISyncDependency*>(dependency))
        {
            if (!vulkanDependency->bPending)
            {
                it = PendingDependencies.erase(it);
                continue;
            }
        }

        ++it;
    }
}

void VulkanRHISyncPointManager::Recycle_NoLock(RHI::RHISyncPoint* syncPoint)
{
    auto it = std::find(PendingSyncPoints.begin(), PendingSyncPoints.end(), syncPoint);
    if (it != PendingSyncPoints.end())
    {
        PendingSyncPoints.erase(it);
    }

    if (auto vulkanSyncPoint = dynamic_cast<VulkanRHISyncPoint*>(syncPoint))
    {
        vulkanSyncPoint->bPending = false;
        vulkanSyncPoint->Fence = nullptr;
        vulkanSyncPoint->Owner = this;
    }
    FreeSyncPoints.push_back(syncPoint);
}

void VulkanRHISyncPointManager::Recycle_NoLock(RHI::RHISyncDependency* dependency)
{
    auto it = std::find(PendingDependencies.begin(), PendingDependencies.end(), dependency);
    if (it != PendingDependencies.end())
    {
        PendingDependencies.erase(it);
    }

    if (auto* vulkanDependency = dynamic_cast<VulkanRHISyncDependency*>(dependency))
    {
        if (vulkanDependency->Semaphore && vulkanDependency->bOwnsSemaphore)
        {
            if (auto* semaphoreManager = Device ? Device->GetSemaphoreManager() : nullptr)
            {
                semaphoreManager->Release(vulkanDependency->Semaphore);
            }
        }
        vulkanDependency->bPending = false;
        vulkanDependency->Semaphore = nullptr;
        vulkanDependency->bOwnsSemaphore = false;
        vulkanDependency->Owner = this;
    }
    FreeDependencies.push_back(dependency);
}

// -------------------------------------------------------------------------------------------------
// VulkanRHISyncPoint Implementation
// -------------------------------------------------------------------------------------------------

void VulkanRHISyncPoint::Activate(
    RHI::EQueueType queueType,
    VulkanFence* inFence,
    uint64_t inValue,
    VulkanRHISyncPointManager* inOwner)
{
    Owner = inOwner;
    Fence = inFence;
    bPending = true;
    Type = queueType;
    Value = inValue;
}

bool VulkanRHISyncPoint::IsReached() const
{
    bool reached = false;
    if (Fence)
    {
        reached = Fence->IsSignaled();
    }
    if (reached && Owner)
    {
        Owner->TryRecycle(const_cast<VulkanRHISyncPoint*>(this));
    }
    return reached;
}

void VulkanRHISyncPoint::Wait() const
{
    if (Fence)
    {
        Fence->Wait();
    }
    if (Owner)
    {
        Owner->TryRecycle(const_cast<VulkanRHISyncPoint*>(this));
    }
}

void VulkanRHISyncDependency::Activate(
    RHI::EQueueType queueType,
    VulkanSemaphore* inSemaphore,
    bool inOwnsSemaphore,
    uint64_t inValue,
    VulkanRHISyncPointManager* inOwner)
{
    Owner = inOwner;
    Semaphore = inSemaphore;
    bOwnsSemaphore = inOwnsSemaphore;
    bPending = true;
    Type = queueType;
    Value = inValue;
}

}
