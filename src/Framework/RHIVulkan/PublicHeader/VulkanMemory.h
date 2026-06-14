#pragma once
#include "VulkanDevice.h"
#include <vector>
#include <cstdint>
#include <memory>
#include <deque>
#include "RHIResource.h"
namespace RHIVulkan{


class VulkanAllocation
{
public:
    VulkanAllocation(VkDeviceMemory memory = VK_NULL_HANDLE, VkDeviceSize offset = 0, VkDeviceSize size = 0, void* mapped = nullptr, VkBuffer buffer = VK_NULL_HANDLE)
        : memory_(memory), offset_(offset), size_(size), mapped_(mapped), buffer_(buffer) {}

    VkDeviceMemory GetMemory() const { return memory_; }
    VkDeviceSize GetOffset() const { return offset_; }
    VkDeviceSize GetSize() const { return size_; }
    void* GetMappedPointer() const { return mapped_; }
    VkBuffer GetBufferHandle() const { return buffer_; }

    void SetBufferHandle(VkBuffer buffer) { buffer_ = buffer; }

private:
    VkDeviceMemory memory_;
    VkDeviceSize offset_;
    VkDeviceSize size_;
    void* mapped_; // 可选，仅对HOST_VISIBLE内存使用
    VkBuffer buffer_; // 关联的缓冲区句柄
};

class VulkanMemoryBlock
{
public:
    VulkanMemoryBlock(VulkanDevice* device, uint32_t memoryTypeIndex, VkDeviceSize blockSize, bool mappable);
    ~VulkanMemoryBlock();

    bool Allocate(VkDeviceSize size, VkDeviceSize alignment, VulkanAllocation& outAlloc);
    void Reset(); // 可选：用于帧结束时重置分配
    bool Free(VulkanAllocation& alloc); // 可选：用于释放分配的内存

    uint32_t GetMemoryTypeIndex() const { return memoryTypeIndex_; }

private:
    VulkanDevice* device_;
    VkDeviceMemory memory_;
    VkDeviceSize size_;
    VkDeviceSize offset_;
    void* mapped_;
    uint32_t memoryTypeIndex_;
    bool mappable_;
};

class VulkanMemoryManager
{
public:
    VulkanMemoryManager(VulkanDevice* device);
    ~VulkanMemoryManager();

    bool Allocate(const VkMemoryRequirements& memReqs, VkMemoryPropertyFlags props, VulkanAllocation& outAlloc);
    void Reset(); // 重置所有块，释放已分配空间（适用于线性分配）
    bool Free(VulkanAllocation& alloc); // 释放分配的内存
private:
    VulkanDevice* device_;
    std::vector<std::unique_ptr<VulkanMemoryBlock>> blocks_;

    uint32_t FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const;
};

class VulkanStagingBuffer : public RHI::RHIStagingBuffer
{
public:
    VulkanStagingBuffer(VulkanDevice* device, VulkanMemoryManager* memManager, VkDeviceSize size);
    ~VulkanStagingBuffer();

    VkBuffer GetHandle() const { return buffer_; }
    VkDeviceSize GetSize() const { return size_; }
    VulkanAllocation& GetAllocation() { return allocation_; }
    void* Map(uint32_t Offset, uint32_t NumBytes) override;
    void Unmap() override;

private:
    VulkanDevice* device_;
    VulkanMemoryManager* memManager_;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VulkanAllocation allocation_;
    void* mapped_ = nullptr;
    VkDeviceSize size_ = 0;
};

class VulkanCommandBuffer;
class VulkanFence;
class VulkanStagingManager
{
public:
    VulkanStagingManager(VulkanDevice* device, VulkanMemoryManager* memManager);
    ~VulkanStagingManager();

    std::shared_ptr<VulkanStagingBuffer> Acquire(VkDeviceSize size);

    // CPU 用完，但 GPU 可能还在用
    void ReleaseToCmdBuffer(VulkanCommandBuffer* cmd,
        std::shared_ptr<VulkanStagingBuffer> buffer);

    void MarkMappedBuffersUsed(std::shared_ptr<VulkanStagingBuffer> buffer,bool used);


    // 每帧调用，检查 fence 并回收 staging
    void GarbageCollect();

private:


    struct PendingEntry
    {
        std::vector<std::shared_ptr<VulkanStagingBuffer>> pendingBuffers;
    };

    VulkanDevice* device_;
    VulkanMemoryManager* memManager_;

    std::vector<std::shared_ptr<VulkanStagingBuffer>> freeBuffers_;
    std::vector<std::shared_ptr<VulkanStagingBuffer>> mapedToFreeBuffers_;
    std::unordered_map<std::shared_ptr<VulkanStagingBuffer>, bool> mapedToFreeBufferUsed_;
    // 记录某个 cmdBuffer 本次录制用了哪些 staging
    std::unordered_map<VulkanCommandBuffer*, PendingEntry> pendingBufferMap_;

    std::mutex mutex_;
};

}