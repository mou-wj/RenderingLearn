#pragma once
#include "VulkanDevice.h"
#include <vector>
#include <cstdint>
#include <memory>
namespace RHIVulkan{


class VulkanAllocation
{
public:
    VulkanAllocation(VkDeviceMemory memory = VK_NULL_HANDLE, VkDeviceSize offset = 0, VkDeviceSize size = 0, void* mapped = nullptr)
        : memory_(memory), offset_(offset), size_(size), mapped_(mapped) {}

    VkDeviceMemory GetMemory() const { return memory_; }
    VkDeviceSize GetOffset() const { return offset_; }
    VkDeviceSize GetSize() const { return size_; }
    void* GetMappedPointer() const { return mapped_; }

private:
    VkDeviceMemory memory_;
    VkDeviceSize offset_;
    VkDeviceSize size_;
    void* mapped_; // 可选，仅对HOST_VISIBLE内存使用
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

}