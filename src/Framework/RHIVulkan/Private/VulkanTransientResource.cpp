#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#include "VulkanTransientResource.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"

namespace RHIVulkan {

    VulkanTransientHeap::Block*
        VulkanTransientHeap::Allocate(
            uint64_t requestSize,
            uint64_t alignment,
            uint32_t beginIndex,
            uint32_t endIndex)
    {
        for (size_t i = 0; i < Blocks.size(); ++i)
        {
            auto& block = Blocks[i];

            // ============
            // 已有 placement，可 alias
            // ============
            if (block.AliasingCount > 0)
            {
                if (block.EndIndex <= beginIndex &&
                    block.Size >= requestSize)
                {
                    block.AliasingCount++;
                    block.EndIndex =
                        std::max(
                            block.EndIndex,
                            endIndex);

                    return &block;
                }

                continue;
            }

            // ============
            // free region
            // ============

            uint64_t alignedOffset =
                Align(block.Offset, alignment);

            uint64_t padding =
                alignedOffset - block.Offset;

            uint64_t requiredSize =
                padding + requestSize;

            if (requiredSize > block.Size)
                continue;

            // ======
            // split
            // ======

            std::vector<Block> newBlocks;

            // left padding
            if (padding > 0)
            {
                newBlocks.push_back({
                    block.Offset,
                    padding,
                    0,
                    0
                    });
            }

            // used block
            Block usedBlock;
            usedBlock.Offset = alignedOffset;
            usedBlock.Size = requestSize;
            usedBlock.EndIndex = endIndex;
            usedBlock.AliasingCount = 1;

            newBlocks.push_back(usedBlock);

            // remain
            uint64_t remain =
                block.Size - requiredSize;

            if (remain > 0)
            {
                newBlocks.push_back({
                    alignedOffset +
                        requestSize,
                    remain,
                    0,
                    0
                    });
            }

            auto iter =
                Blocks.begin() + i;

            iter = Blocks.erase(iter);

            iter = Blocks.insert(
                iter,
                newBlocks.begin(),
                newBlocks.end());

            // used block index
            size_t usedIndex =
                (padding > 0) ? 1 : 0;

            return &(*(iter + usedIndex));
        }

        return nullptr;
    }




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
    void VulkanTransientHeap::Release(
        uint64_t offset,
        uint64_t size)
    {
        for (auto& block : Blocks)
        {
            if (block.Offset != offset)
                continue;

            if (block.Size != size)
                continue;

            assert(
                block.AliasingCount > 0);

            block.AliasingCount--;

            if (block.AliasingCount == 0)
            {
                block.EndIndex = 0;
            }

            break;
        }

        MergeAdjacent();
    }
    void VulkanTransientHeap::MergeAdjacent()
    {
        if (Blocks.empty())
            return;

        for (size_t i = 0;
            i + 1 < Blocks.size();)
        {
            auto& current =
                Blocks[i];

            auto& next =
                Blocks[i + 1];

            bool canMerge =
                current.AliasingCount == 0 &&
                next.AliasingCount == 0 &&
                current.Offset +
                current.Size ==
                next.Offset;

            if (!canMerge)
            {
                ++i;
                continue;
            }

            current.Size += next.Size;

            Blocks.erase(
                Blocks.begin() + i + 1);
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

        heap->Blocks.push_back({ 0, size, 0, 0 });

        Heaps.push_back(std::move(heap));
        return Heaps.back().get();
    }

    // =========================
    // Allocation
    // =========================

    VulkanTransientAllocation VulkanTransientResourceManager::Allocate(
            uint64_t size,
            uint64_t alignment,
            uint32_t beginIndex,
            uint32_t endIndex,
            uint32_t memTypeIndex)
    {
        for (auto& heap : Heaps)
        {
            if (heap->MemoryTypeIndex != memTypeIndex)
                continue;

            auto* block =
                heap->Allocate(
                    size,
                    alignment,
                    beginIndex,
                    endIndex);

            if (block)
            {
                return {
                    heap.get(),
                    block->Offset,
                    block->Size
                };
            }
        }

        // ❗ 不够 → 新建 heap
        auto* newHeap = CreateHeap(std::max(size, DEFAULT_HEAP_SIZE), memTypeIndex);
        auto* block = newHeap->Allocate(size, alignment, beginIndex,endIndex);
        return {
                newHeap,
                block->Offset,
                block->Size
        };
    }

    // =========================
    // Free (logical)
    // =========================

    void VulkanTransientResourceManager::Free(
        VulkanTransientAllocation& alloc)
    {
        auto* heap = alloc.Heap;
        heap->Release(alloc.Offset, alloc.Size);
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
        auto alloc = Allocate(memReq.size, memReq.alignment, beginIndex,endIndex, memTypeIndex);

        VKFunc::BindImageMemory(
            Device->GetHandle(),
            texture->GetImage(),
            alloc.Heap->Memory,
            alloc.Offset);
        
        auto transient = std::make_shared<RHI::RHITransientTexture>(texture);
        texture->SetTransientAllocation(alloc);
        texture->CreateDefaultView();

        return transient;
    }

    // =========================
    // Release Texture
    // =========================

    void VulkanTransientResourceManager::ReleaseTransientTexture(
        const RHI::RHITransientTexture* texture)
    {
        auto vkTex = static_cast<VulkanTexture*>(texture->GetTexture().get());
        auto alloc = vkTex->GetTransientAllocation();
        Free(alloc);
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
        auto alloc = Allocate(memReq.size, memReq.alignment, beginIndex,endIndex, memTypeIndex);

        VKFunc::BindBufferMemory(
            Device->GetHandle(),
            vkBuffer->GetHandle(),
            alloc.Heap->Memory,
            alloc.Offset);
        vkBuffer->SetTransientAllocation(alloc);


        auto transient = std::make_shared<RHI::RHITransientBuffer>(vkBuffer);


        return transient;
    }

    void VulkanTransientResourceManager::ReleaseTransientBuffer(
        const RHI::RHITransientBuffer* buffer)
    {
        auto vkBuf = static_cast<VulkanBuffer*>(buffer->GetBuffer().get());

        auto alloc = vkBuf->GetTransientAllocation();
        Free(alloc);
    }
} // namespace WR::RHIVulkan