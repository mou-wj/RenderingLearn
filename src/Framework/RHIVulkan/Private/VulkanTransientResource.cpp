#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#include "VulkanTransientResource.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"

namespace RHIVulkan {

    VulkanTransientResourceManager::VulkanTransientResourceManager(VulkanDevice* device)
        : Device(device)
    {
    }

    VulkanTransientResourceManager::~VulkanTransientResourceManager()
    {
        VkDevice vkDevice = Device->GetHandle();

        for (auto& heap : Heaps)
        {
            if (heap->Memory != VK_NULL_HANDLE)
            {
                VKFunc::FreeMemory(vkDevice, heap->Memory);
            }
        }
    }

    // =========================
    // Utils
    // =========================

    uint64_t VulkanTransientResourceManager::Align(uint64_t value, uint64_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    // =========================
    // Heap Creation
    // =========================

    VulkanTransientHeap* VulkanTransientResourceManager::CreateHeap(uint64_t size, uint32_t memoryTypeIndex)
    {
        auto heap = std::make_unique<VulkanTransientHeap>();

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        VKFunc::AllocateMemory(Device->GetHandle(), &allocInfo, &heap->Memory);

        heap->Size = size;
        heap->MemoryTypeIndex = memoryTypeIndex;

        heap->Blocks.push_back({ 0, size, 0, true });

        Heaps.push_back(std::move(heap));
        return Heaps.back().get();
    }

    // =========================
    // Allocation
    // =========================

    VulkanTransientAllocation VulkanTransientResourceManager::Allocate(
            uint64_t size,
            uint64_t alignment,
            uint32_t beginIndex,uint32_t memTypeIndex)
    {
        for (auto& heap : Heaps)
        {
            for (auto& block : heap->Blocks)
            {
                if (!block.bFree)
                    continue;

                if (block.FreeAfterIndex > beginIndex)
                    continue;

                uint64_t alignedOffset = Align(block.Offset, alignment);

                if (alignedOffset + size <= block.Offset + block.Size)
                {
                    block.bFree = false;

                    return { heap.get(), alignedOffset, size };
                }
            }
        }

        // ❗ 不够 → 新建 heap
        auto* newHeap = CreateHeap(std::max(size, DEFAULT_HEAP_SIZE), memTypeIndex);
        return Allocate(size, alignment, beginIndex, memTypeIndex);
    }

    // =========================
    // Free (logical)
    // =========================

    void VulkanTransientResourceManager::Free(
        VulkanTransientAllocation& alloc,
        uint32_t endIndex)
    {
        auto* heap = alloc.Heap;

        for (auto& block : heap->Blocks)
        {
            if (block.Offset == alloc.Offset)
            {
                block.bFree = true;
                block.FreeAfterIndex = endIndex;
                break;
            }
        }
    }

    // =========================
    // Create Texture
    // =========================

    RHI::RHITransientTextureSP
        VulkanTransientResourceManager::CreateTransientTexture(
            const RHI::RHITextureDesc& desc,
            uint32_t beginIndex,
            uint32_t endIndex)
    {

        auto texture = std::make_shared<VulkanTexture>(Device, desc,true);
        VkMemoryRequirements memReq;
        VKFunc::GetImageMemoryRequirements(Device->GetHandle(), texture->GetImage(), &memReq);
        uint32_t memTypeIndex = Device->FindMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        auto alloc = Allocate(memReq.size, memReq.alignment, beginIndex, memTypeIndex);

        VKFunc::BindImageMemory(
            Device->GetHandle(),
            texture->GetImage(),
            alloc.Heap->Memory,
            alloc.Offset);
        
        auto transient = std::make_shared<RHI::RHITransientTexture>(texture);
        texture->SetTransientAllocation(alloc);
        transient->Acquire("TransientTex", beginIndex);
        transient->Release(endIndex);

        return transient;
    }

    // =========================
    // Release Texture
    // =========================

    void VulkanTransientResourceManager::ReleaseTransientTexture(
        const RHI::RHITransientTexture* texture,
        uint32_t endIndex)
    {
        auto vkTex = static_cast<VulkanTexture*>(texture->GetTexture().get());
        auto alloc = vkTex->GetTransientAllocation();
        Free(alloc, endIndex);
    }

    // =========================
    // Buffer（同理简化版）
    // =========================

    RHI::RHITransientBufferSP
        VulkanTransientResourceManager::CreateTransientBuffer(
            const RHI::RHIBufferDesc& desc,
            uint32_t beginIndex,
            uint32_t endIndex)
    {
        auto vkBuffer = std::make_shared<VulkanBuffer>(Device, desc,true);

        VkMemoryRequirements memReq;
        VKFunc::GetBufferMemoryRequirements(Device->GetHandle(), vkBuffer->GetHandle(), &memReq);
        uint32_t memTypeIndex = Device->FindMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        auto alloc = Allocate(memReq.size, memReq.alignment, beginIndex, memTypeIndex);

        VKFunc::BindBufferMemory(
            Device->GetHandle(),
            vkBuffer->GetHandle(),
            alloc.Heap->Memory,
            alloc.Offset);
        vkBuffer->SetTransientAllocation(alloc);


        auto transient = std::make_shared<RHI::RHITransientBuffer>(vkBuffer);

        transient->Acquire("TransientBuf", beginIndex);
        transient->Release(endIndex);

        return transient;
    }

    void VulkanTransientResourceManager::ReleaseTransientBuffer(
        const RHI::RHITransientBuffer* buffer,
        uint32_t endIndex)
    {
        auto vkBuf = static_cast<VulkanBuffer*>(buffer->GetBuffer().get());

        auto alloc = vkBuf->GetTransientAllocation();
        Free(alloc, endIndex);
    }
} // namespace WR::RHIVulkan