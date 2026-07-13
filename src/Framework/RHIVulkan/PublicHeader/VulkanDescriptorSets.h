#pragma once
#include "VulkanDevice.h"
#include "VulkanFuncWrapper.h"
#include <vector>
#include <stdexcept>
#include <map>
#include <array>
#include <set>
#include <list>
#include <unordered_map>
#define VK_DESCRIPTOR_TYPE_RANGE_SIZE (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1)

namespace RHIVulkan {
    class VulkanCommandBuffer;
    //------------------------------------------------------------
    // Descriptor Set Binding / Layout
    //------------------------------------------------------------
    struct DescriptorSetBinding
    {
        uint32_t Binding = 0;
        VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        uint32_t Count = 1;
        VkShaderStageFlags StageFlags = 0;
    };


    //------------------------------------------------------------
    // Layout 描述信息（Hash + Bindings）
    //------------------------------------------------------------
    struct DescriptorSetLayoutInfo
    {
        std::vector<DescriptorSetBinding> bindings;
        static uint64_t CalculateHash(const DescriptorSetLayoutInfo& info);
        void AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags stageFlags);
    };

    //------------------------------------------------------------
    // Layout 句柄 + Info
    //------------------------------------------------------------
    struct DescriptorSetLayoutHandle
    {
        VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
        DescriptorSetLayoutInfo Info;
    };

    //------------------------------------------------------------
    // Vulkan Descriptor Pool 封装
    //------------------------------------------------------------
    class VulkanDescriptorPool
    {
    public:
        VulkanDescriptorPool(VulkanDevice* device, uint32_t maxSets, const std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE>& poolSizes);
        ~VulkanDescriptorPool();

        bool AllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& outSet);
        void Reset();

    private:
        void CreatePool();

    private:
        friend class TypedDescriptorPool;
		friend class VulkanDescriptorSetManager;
        VulkanDevice* Device;
        uint32_t MaxDescriptorSets;
        std::array<float, VK_DESCRIPTOR_TYPE_RANGE_SIZE> PoolSizes{};
        VkDescriptorPool Pool = VK_NULL_HANDLE;
        uint32_t AllocatedCount = 0;
    };

    class TypedDescriptorPool
    {
    public:
        TypedDescriptorPool(
            VulkanDevice* device,
            VkDescriptorSetLayout layout,
            const DescriptorSetLayoutInfo& layoutInfo,
            uint32_t maxSetsPerPool);
        ~TypedDescriptorPool();

        VkDescriptorSet Allocate();
    private:
        VulkanDevice* Device;
        VkDescriptorSetLayout Layout;
        DescriptorSetLayoutInfo LayoutInfo;
        uint32_t MaxSetsPerPool;
        friend class VulkanDescriptorSetManager;
        std::vector<std::unique_ptr<VulkanDescriptorPool>> Pools;
    };
 

    //------------------------------------------------------------
    // Layout Cache 管理 VkDescriptorSetLayout
    //------------------------------------------------------------
    class VulkanDescriptorSetLayoutManager
    {
    public:
        VulkanDescriptorSetLayoutManager(VulkanDevice* device);
        ~VulkanDescriptorSetLayoutManager();
        VkDescriptorSetLayout GetOrCreateLayout(const DescriptorSetLayoutInfo& info);

    private:
        VulkanDevice* Device;
        std::unordered_map<uint64_t, VkDescriptorSetLayout> LayoutMap;
    };



    //------------------------------------------------------------
    // 全局 DescriptorSet Cache 管理器
    //------------------------------------------------------------
    class VulkanDescriptorSetManager
    {
    public:
        VulkanDescriptorSetManager(VulkanDevice* device, VulkanDescriptorSetLayoutManager* layoutManager);
        ~VulkanDescriptorSetManager();

        VkDescriptorSet GetDescriptorSet(const DescriptorSetLayoutInfo& layoutInfo);
        void BindDescriptorSets(VulkanCommandBuffer* cmdBuffer, VkPipelineLayout layout, VkPipelineBindPoint pipelineBindingPoint, const std::vector<VkDescriptorSet>& descriptorSets, const std::vector<uint32_t>& dynamicOffsets = {});
		void GarbageCollect();
        
    private:
        struct PendingDescriptorSetsInfo {
            std::vector<VkDescriptorSet> pendingSets;
            std::vector<uint32_t> pendingDynamicOffsets;
            VulkanCommandBuffer* CmdBuffer = nullptr;
        };
        struct AllocatedSets {
            std::set<VkDescriptorSet> InUseSets;
            std::set<VkDescriptorSet> FreeSets;
        };

        VulkanDevice* Device;
        VulkanDescriptorSetLayoutManager* LayoutCache;
        std::unordered_map<uint64_t, AllocatedSets> LayoutAllocatedSetsMap;
        std::unordered_map<VkDescriptorSet, uint64_t> DescriptorSetLayoutHashMap;

        std::list<PendingDescriptorSetsInfo> PendingFreeSetInfos;
        std::unordered_map<uint64_t, std::unique_ptr<TypedDescriptorPool>> LayoutDescriptorPoolMap;
    };

    class VulkanDescriptorWriter
    {
    public:
        void Reset()
        {
            Writes.clear();
            ImageInfos.clear();
            BufferInfos.clear();
            Dirty = false;
        }

        void WriteImage(VkDescriptorSet set,
            uint32_t binding,
            uint32_t element,
            VkDescriptorType type,
            VkImageView view,
            VkImageLayout layout,
            VkSampler sampler = VK_NULL_HANDLE)
        {
            VkDescriptorImageInfo& info = ImageInfos.emplace_back();
            info.imageView = view;
            info.imageLayout = layout;
            info.sampler = sampler;

            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = set;
            write.dstBinding = binding;
            write.dstArrayElement = element;
            write.descriptorType = type;
            write.descriptorCount = 1;
            write.pImageInfo = &info;

            Writes.push_back(write);
            Dirty = true;
        }

        void WriteBuffer(VkDescriptorSet set,
            uint32_t binding,
            uint32_t element,
            VkDescriptorType type,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize range)
        {
            VkDescriptorBufferInfo& info = BufferInfos.emplace_back();
            info.buffer = buffer;
            info.offset = offset;
            info.range = range;

            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = set;
            write.dstBinding = binding;
            write.dstArrayElement = element;
            write.descriptorType = type;
            write.descriptorCount = 1;
            write.pBufferInfo = &info;

            Writes.push_back(write);
            Dirty = true;
        }
        void WriteTexelBuffer(VkDescriptorSet set,uint32_t binding, uint32_t element, VkDescriptorType type, VkBufferView view)
        {
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = set;
            write.dstBinding = binding;
            write.dstArrayElement = element;
            write.descriptorType = type;
            write.descriptorCount = 1;
            BufferViews.push_back(view);
            write.pTexelBufferView = &BufferViews.back();
            Writes.push_back(write);
            Dirty = true;
        }

        void Update(VkDevice device)
        {

            VKFunc::UpdateDescriptorSets(device,
                (uint32_t)Writes.size(),
                Writes.data(),
                0, nullptr);

            Dirty = false;
        }

        bool IsDirty() const { return Dirty; }

    private:
        std::vector<VkWriteDescriptorSet> Writes;
        std::deque<VkDescriptorImageInfo> ImageInfos;
        std::deque<VkDescriptorBufferInfo> BufferInfos;
        std::deque<VkBufferView> BufferViews;
        bool Dirty = false;
    };


} // namespace WR::RHIVulkan
