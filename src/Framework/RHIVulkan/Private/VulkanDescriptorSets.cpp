#include "VulkanDescriptorSets.h"
#include <stdexcept>
namespace RHIVulkan {
        //------------------------------------------------------------
        // DescriptorSetLayoutInfo
        //------------------------------------------------------------
    uint64_t DescriptorSetLayoutInfo::CalculateHash()
    {
        if (Hash != 0) return Hash;

        uint64_t h = 14695981039346656037ull;
        for (auto& b : Bindings)
        {
            h ^= b.Binding; h *= 1099511628211ull;
            h ^= static_cast<uint64_t>(b.Type); h *= 1099511628211ull;
            h ^= b.Count; h *= 1099511628211ull;
            h ^= b.StageFlags; h *= 1099511628211ull;
        }
        Hash = h;
        return Hash;
    }

    void DescriptorSetLayoutInfo::AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags stageFlags)
    {
        Bindings.push_back({ binding, type, count, stageFlags });
        Hash = 0; // 重新计算 hash
    }

    //------------------------------------------------------------
    // VulkanDescriptorPool
    //------------------------------------------------------------
    VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* device, uint32_t maxSets, const std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE>& poolSizes)
        : Device(device), MaxDescriptorSets(maxSets)
    {
        for (int i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
            PoolSizes[i] = static_cast<float>(poolSizes[i]);

        CreatePool();
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        if (Pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(Device->GetHandle(), Pool, nullptr);
            Pool = VK_NULL_HANDLE;
        }
    }

    void VulkanDescriptorPool::CreatePool()
    {
        std::vector<VkDescriptorPoolSize> sizes;
        for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
        {
            if (PoolSizes[i] > 0)
            {
                VkDescriptorPoolSize s{};
                s.type = static_cast<VkDescriptorType>(i);
                s.descriptorCount = static_cast<uint32_t>(PoolSizes[i]);
                sizes.push_back(s);
            }
        }

        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = static_cast<uint32_t>(sizes.size());
        info.pPoolSizes = sizes.data();
        info.maxSets = MaxDescriptorSets;
        info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (vkCreateDescriptorPool(Device->GetHandle(), &info, nullptr, &Pool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create VulkanDescriptorPool");
        }
        AllocatedCount = 0;
    }

    bool VulkanDescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& outSet)
    {
        if (AllocatedCount >= MaxDescriptorSets)
            return false;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = Pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkResult res = vkAllocateDescriptorSets(Device->GetHandle(), &allocInfo, &outSet);
        if (res == VK_SUCCESS)
        {
            ++AllocatedCount;
            return true;
        }
        return false;
    }

    void VulkanDescriptorPool::Reset()
    {
        if (Pool != VK_NULL_HANDLE)
        {
            vkResetDescriptorPool(Device->GetHandle(), Pool, 0);
            AllocatedCount = 0;
        }
    }

    //------------------------------------------------------------
    // TypedDescriptorPool
    //------------------------------------------------------------
    TypedDescriptorPool::TypedDescriptorPool(VulkanDevice* device, VkDescriptorSetLayout layout, const DescriptorSetLayoutInfo& layoutInfo, uint32_t maxSetsPerPool)
        : Device(device), Layout(layout), LayoutInfo(layoutInfo), MaxSetsPerPool(maxSetsPerPool)
    {
    }

    VkDescriptorSet TypedDescriptorPool::Allocate()
    {
        for (auto& pool : Pools)
        {
            VkDescriptorSet set = VK_NULL_HANDLE;
            if (pool->AllocateDescriptorSet(Layout, set))
                return set;
        }

        // 没有可用 pool，创建新的
        std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE> poolSizes{};
        for (auto& b : LayoutInfo.Bindings)
        {
            poolSizes[b.Type] += b.Count;
        }

        Pools.push_back(std::make_unique<VulkanDescriptorPool>(Device, MaxSetsPerPool, poolSizes));

        VkDescriptorSet set = VK_NULL_HANDLE;
        if (!Pools.back()->AllocateDescriptorSet(Layout, set))
            throw std::runtime_error("TypedDescriptorPool failed to allocate descriptor set");

        return set;
    }

    //------------------------------------------------------------
    // DescriptorSetLayoutManager
    //------------------------------------------------------------
    VulkanDescriptorSetLayoutManager::VulkanDescriptorSetLayoutManager(VulkanDevice* device)
        : Device(device)
    {
    }

    VkDescriptorSetLayout VulkanDescriptorSetLayoutManager::GetOrCreateLayout(const DescriptorSetLayoutInfo& info)
    {
		
        uint64_t hash = info.Hash;
        auto it = LayoutMap.find(hash);
        if (it != LayoutMap.end())
            return it->second;

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto& b : info.Bindings)
        {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = b.Binding;
            binding.descriptorType = b.Type;
            binding.descriptorCount = b.Count;
            binding.stageFlags = b.StageFlags;
            binding.pImmutableSamplers = nullptr;
            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(Device->GetHandle(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create VkDescriptorSetLayout");

        LayoutMap[hash] = layout;
        return layout;
    }

    //------------------------------------------------------------
    // DescriptorSetManager
    //------------------------------------------------------------
    VulkanDescriptorSetManager::VulkanDescriptorSetManager(VulkanDevice* device, VulkanDescriptorSetLayoutManager* layoutManager)
        : Device(device), LayoutCache(layoutManager)
    {
    }

    DescriptorSetEntry VulkanDescriptorSetManager::GetDescriptorSets(const DescriptorSetLayoutInfo& layoutInfo)
    {
        uint64_t hash = layoutInfo.Hash;
        auto it = LayoutDescriptorPoolMap.find(hash);
        if (it == LayoutDescriptorPoolMap.end())
        {
            VkDescriptorSetLayout layout = LayoutCache->GetOrCreateLayout(layoutInfo);
            LayoutDescriptorPoolMap[hash] = std::make_unique<TypedDescriptorPool>(Device, layout, layoutInfo, 256);
            it = LayoutDescriptorPoolMap.find(hash);
        }

        VkDescriptorSet set = it->second->Allocate();

        return { set, hash, it->second.get() };
    }

} // namespace WR::RHIVulkan
