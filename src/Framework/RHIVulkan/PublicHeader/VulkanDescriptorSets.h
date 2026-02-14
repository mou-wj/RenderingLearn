#pragma once
#include "VulkanDevice.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <map>
#include <array>
#define VK_DESCRIPTOR_TYPE_RANGE_SIZE (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1)

namespace RHIVulkan {

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
        std::vector<DescriptorSetBinding> Bindings;
        uint64_t Hash = 0;

        uint64_t CalculateHash();
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

        VkDescriptorSet Allocate();

    private:
        VulkanDevice* Device;
        VkDescriptorSetLayout Layout;
        DescriptorSetLayoutInfo LayoutInfo;
        uint32_t MaxSetsPerPool;

        std::vector<std::unique_ptr<VulkanDescriptorPool>> Pools;
    };
 

    //------------------------------------------------------------
    // Layout Cache 管理 VkDescriptorSetLayout
    //------------------------------------------------------------
    class VulkanDescriptorSetLayoutManager
    {
    public:
        VulkanDescriptorSetLayoutManager(VulkanDevice* device);
        VkDescriptorSetLayout GetOrCreateLayout(const DescriptorSetLayoutInfo& info);

    private:
        VulkanDevice* Device;
        std::unordered_map<uint64_t, VkDescriptorSetLayout> LayoutMap;
    };

    struct DescriptorSetEntry
    {
        VkDescriptorSet Set = VK_NULL_HANDLE;
        uint64_t LayoutHash = 0;
        // 归属 TypedPool（用于回收）
        class TypedDescriptorPool* OwnerPool = nullptr;
    };

    //------------------------------------------------------------
    // 全局 DescriptorSet Cache 管理器
    //------------------------------------------------------------
    class VulkanDescriptorSetManager
    {
    public:
        VulkanDescriptorSetManager(VulkanDevice* device, VulkanDescriptorSetLayoutManager* layoutManager);

        DescriptorSetEntry GetDescriptorSets(const DescriptorSetLayoutInfo& layoutInfo);
                
    private:
        VulkanDevice* Device;
        VulkanDescriptorSetLayoutManager* LayoutCache;
        std::unordered_map<uint64_t, std::unique_ptr<TypedDescriptorPool>> LayoutDescriptorPoolMap;
    };

} // namespace WR::RHIVulkan
