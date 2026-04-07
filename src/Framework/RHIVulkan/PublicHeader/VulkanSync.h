#pragma once
#include "VulkanDevice.h"
#include "RHICommandContex.h"
#include <queue>
#include <mutex>
#include <deque>
#include <vector>
#include <memory>

namespace RHIVulkan{

class VulkanDevice;
class VulkanCommandBuffer;
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

    // �أ����пɸ��ö���
    std::queue<std::shared_ptr<VulkanEvent>> pool_;

    // �������д����Ķ������������͵��ԣ�
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

    // requireUnsignaled=true: always create a new unsignaled semaphore.
    // requireUnsignaled=false: reuse from pool when available.
    VulkanSemaphore* Acquire(bool requireUnsignaled = false);
    void Release(VulkanSemaphore* sem);

private:
    VulkanDevice* device_;

    static constexpr size_t MaxSemaphorePoolSize = 100;
    std::queue<VulkanSemaphore*> pool_;

    // �������д�������
    std::vector<VulkanSemaphore*> managedObjects_;

    std::mutex mutex_;
};

// Vulkan Fence
class VulkanFence {
public:
    VulkanFence(VulkanDevice* device);
    ~VulkanFence();
    VkFence GetHandle() const { return Fence; }

    bool IsSignaled() const;
    void Reset();
    void Wait();
private:
    VkFence Fence = VK_NULL_HANDLE;
    VulkanDevice* Device = nullptr;
};

class VulkanFenceManager
{
public:
    VulkanFenceManager(VulkanDevice* device);
    ~VulkanFenceManager();

    VulkanFence* AcquireFence();
    void ReleaseFence(VulkanFence* fence);
    void GarbageCollect();

private:
    VulkanDevice* device = nullptr;
    std::vector<std::unique_ptr<VulkanFence>> allFences;
    std::deque<VulkanFence*> availableFences;
    std::deque<VulkanFence*> pendingFences;
};

// Vulkan RHI-level Sync Point Manager
class RHIVULKAN_API VulkanRHISyncPointManager
{
public:
    explicit VulkanRHISyncPointManager(VulkanDevice* device);
    ~VulkanRHISyncPointManager();

    // Acquire a completion point for the given fence (fence is managed by device's FenceManager)
    RHI::RHISyncPoint* AcquireCompletion(
        RHI::EQueueType queueType,
        VulkanFence* fence);
    RHI::RHISyncDependency* AcquireDependency(
        RHI::EQueueType queueType,
        VulkanSemaphore* semaphore,
        bool ownsSemaphore = false);
    void GarbageCollect();
    void TryRecycle(RHI::RHISyncPoint* syncPoint);
    void TryRecycle(RHI::RHISyncDependency* dependency);
    void WaitAndRecycleAll();

private:
    friend class VulkanRHISyncPoint;
    friend class VulkanRHISyncDependency;
    
    void GarbageCollect_NoLock();
    void Recycle_NoLock(RHI::RHISyncPoint* syncPoint);
    void Recycle_NoLock(RHI::RHISyncDependency* dependency);

    VulkanDevice* Device = nullptr;
    uint64_t NextValue = 1;
    std::vector<std::unique_ptr<RHI::RHISyncPoint>> AllSyncPoints;
    std::vector<RHI::RHISyncPoint*> FreeSyncPoints;
    std::vector<RHI::RHISyncPoint*> PendingSyncPoints;
    std::vector<std::unique_ptr<RHI::RHISyncDependency>> AllDependencies;
    std::vector<RHI::RHISyncDependency*> FreeDependencies;
    std::vector<RHI::RHISyncDependency*> PendingDependencies;
    std::mutex Mutex;
};

// Vulkan RHI-level Sync Point (backed by VulkanFence from device)
class RHIVULKAN_API VulkanRHISyncPoint final : public RHI::RHISyncPoint
{
public:
    VulkanRHISyncPoint() = default;

    bool IsReached() const override;
    void Wait() const override;

private:
    friend class VulkanRHISyncPointManager;
    void Activate(
        RHI::EQueueType queueType,
        VulkanFence* inFence,
        uint64_t inValue,
        VulkanRHISyncPointManager* inOwner);

    VulkanRHISyncPointManager* Owner = nullptr;
    VulkanFence* Fence = nullptr;
    bool bPending = false;
};

class RHIVULKAN_API VulkanRHISyncDependency final : public RHI::RHISyncDependency
{
public:
    VulkanRHISyncDependency() = default;

    VulkanSemaphore* GetSemaphore() const { return Semaphore; }

private:
    friend class VulkanRHISyncPointManager;
    void Activate(
        RHI::EQueueType queueType,
        VulkanSemaphore* inSemaphore,
        bool inOwnsSemaphore,
        uint64_t inValue,
        VulkanRHISyncPointManager* inOwner);

    VulkanRHISyncPointManager* Owner = nullptr;
    VulkanSemaphore* Semaphore = nullptr;
    bool bOwnsSemaphore = false;
    bool bPending = false;
};

}