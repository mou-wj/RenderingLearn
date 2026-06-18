#pragma once
#include "VulkanDevice.h"
#include "RHICommandContex.h"
#include <queue>
#include <mutex>
#include <deque>
#include <vector>
#include <set>
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
    VulkanSemaphore(VulkanDevice* device,bool isBinary = false,uint64_t initialValue = 0);
    ~VulkanSemaphore();

    VkSemaphore GetHandle() const;
    bool IsBinary() const { return isBinary_; }
    // 获取当前 GPU 已经执行到的数值
    uint64_t GetCurrentValue();
    // CPU 端阻塞等待直到达到某个值
    void Wait(uint64_t Value, uint64_t TimeoutNS = UINT64_MAX);

private:
    VulkanDevice* device_;
    VkSemaphore semaphore_;
    bool isBinary_ = false;
    uint64_t value_ = 0;
};

class VulkanSemaphoreManager {
public:
    VulkanSemaphoreManager(VulkanDevice* device);
    ~VulkanSemaphoreManager();
    // binary semaphore manager
    // requireUnsignaled=true: always create a new unsignaled semaphore.
    // requireUnsignaled=false: reuse from pool when available.
    VulkanSemaphore* AcquireBinary();
    void ReleaseBinary(VulkanSemaphore* sem);

private:
    void BatchCreateSemaphores(size_t count);

    VulkanDevice* device_;
    std::mutex mutex_;

    static constexpr size_t BatchSize = 100;

    // 空闲池：存放全新 vkCreate 出来的干净信号量
    std::set<VulkanSemaphore*> freePool_;

    // 已用池：存放外部归还的、确定已空闲的信号量，攒满即砸碎
    std::set<VulkanSemaphore*> allAllocateds_;
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


// Vulkan RHI-level Sync Point (based on VkTimelineSemaphore)
class RHIVULKAN_API VulkanRHISyncPoint final : public RHI::RHISyncPoint {
public:
    VulkanRHISyncPoint(VulkanDevice* device, RHI::EQueueType queueType, uint64_t initialValue = 0,bool isBinary = false);
    VulkanRHISyncPoint(VulkanDevice* device, RHI::EQueueType queueType, VulkanSemaphore* semaphore);
    ~VulkanRHISyncPoint();
    void SetSemaphore(VulkanSemaphore* semaphore) { 
        if (!ownSemaphore_) {
            semaphore_ = semaphore;
            }
        }

    // 获取当前 GPU 已经执行到的数值
    uint64_t GetCurrentValue() override;
    // CPU 端阻塞等待直到达到某个值
    void Wait(uint64_t Value, uint64_t TimeoutNS = UINT64_MAX) override;

    // 获取VkSemaphore句柄（如有需要跨API用）
    VulkanSemaphore* GetSemaphore() const { return semaphore_; }

    bool IsBinary() const { return semaphore_->IsBinary(); }


private:
    bool ownSemaphore_ = true;
    VulkanDevice* device_ = nullptr;
    VulkanSemaphore* semaphore_ = nullptr;
};
}