#pragma once
#include "RHITransientResource.h"
#include "vulkan/vulkan.h"
#include <memory>

namespace RHIVulkan {

class VulkanDevice;
// =========================
    // Heap / Allocation
    // =========================

struct VulkanTransientHeap
{
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    uint64_t Size = 0;
    uint32_t MemoryTypeIndex = 0;  // ✅ 必须加
    struct Block
    {
        uint64_t Offset = 0;
        uint64_t Size = 0;

        uint32_t EndIndex = 0; // 生命周期结束
        uint32_t AliasingCount = 0;
        
    };
public:

    Block* Allocate(
        uint64_t requestSize,
        uint64_t alignment,
        uint32_t beginIndex,
        uint32_t endIndex);

    void Release(
        uint64_t offset,
        uint64_t size);

private:

    void MergeAdjacent();

    uint64_t Align(
        uint64_t value,
        uint64_t alignment) const
    {
        return (value + alignment - 1)
            & ~(alignment - 1);
    }
public:
    std::vector<Block> Blocks;
};

struct VulkanTransientAllocation
{
    VulkanTransientHeap* Heap = nullptr;
    uint64_t Offset = 0;
    uint64_t Size = 0;
};

class RHIVULKAN_API VulkanTransientResourceManager : public RHI::RHITransientResourceManager
{
public:
    explicit VulkanTransientResourceManager(VulkanDevice* device);
    ~VulkanTransientResourceManager();

    // 创建资源（带生命周期）
    RHI::RHITransientTextureSP CreateTransientTexture(
        const RHI::RHITextureDesc& desc,
        uint32_t beginIndex,
        uint32_t endIndex) override;

    RHI::RHITransientBufferSP CreateTransientBuffer(
        const RHI::RHIBufferDesc& desc,
        uint32_t beginIndex,
        uint32_t endIndex) override;

    // 释放（逻辑释放）
    void ReleaseTransientTexture(const RHI::RHITransientTexture* texture) override;
    void ReleaseTransientBuffer(const RHI::RHITransientBuffer* buffer) override;
    VulkanTransientResourceManager(const VulkanTransientResourceManager&) = delete;
    VulkanTransientResourceManager& operator=(const VulkanTransientResourceManager&) = delete;
private:

    
private:

    VulkanTransientAllocation Allocate(
        uint64_t size,
        uint64_t alignment,
        uint32_t beginIndex,
        uint32_t endIndex,
        uint32_t memTypeIndex);

    void Free(
        VulkanTransientAllocation& alloc);

    VulkanTransientHeap* CreateHeap(uint64_t sizem, uint32_t memoryTypeIndex);

    uint64_t Align(uint64_t value, uint64_t alignment);

private:

    VulkanDevice* Device = nullptr;

    std::vector<std::unique_ptr<VulkanTransientHeap>> Heaps;

    static constexpr uint64_t DEFAULT_HEAP_SIZE = 64ull * 1024 * 1024; // 64MB
};

} // namespace WR::RHIVulkan