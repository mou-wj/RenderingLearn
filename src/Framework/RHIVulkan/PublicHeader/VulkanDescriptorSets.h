#pragma once
#include "VulkanDevice.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <map>

namespace RHIVulkan {


// 描述符集封装
class VulkanDescriptorSet
{
public:
    VulkanDescriptorSet(VkDescriptorSet* set, VkDescriptorSetLayout layout,const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    ~VulkanDescriptorSet() = default;

    VkDescriptorSet GetHandle() const { return _set; }
    VkDescriptorSetLayout GetLayout() const { return _layout; }
    const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const { return bindings; }
private:
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    VkDescriptorSet _set = VK_NULL_HANDLE;
    VkDescriptorSetLayout _layout = VK_NULL_HANDLE;
};

using VulkanDescriptorSetLayoutMap = std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>;
using VulkanDescriptorSets = std::vector<VulkanDescriptorSet*>;


// 描述符池封装
class VulkanDescriptorPool
{
public:
    VulkanDescriptorPool(VulkanDevice* device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);
    ~VulkanDescriptorPool();

    VkDescriptorPool GetHandle() const { return _pool; }

    // 分配描述符集
    VulkanDescriptorSets AllocateDescriptorSet(const VulkanDescriptorSetLayoutMap& map);

    // 重置池
    void Reset();

    bool CanAllocate(const std::map<VkDescriptorType, uint32_t>& want) const;

private:
    VkDescriptorSetLayout CreateLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VulkanDevice* _device = nullptr;
    VkDescriptorPool _pool = VK_NULL_HANDLE;
    std::vector<VulkanDescriptorSet*> _allocatedSets; // 已分配的描述符集
    std::map<VkDescriptorType, uint32_t> _validPoolSizes; // 有效的池大小
    std::map<VkDescriptorType, uint32_t> _usedPoolSizes; // 使用的池大小
};

// 描述符池管理器
class VulkanDescriptorPoolManager
{
public:
    VulkanDescriptorPoolManager(VulkanDevice* device);
    ~VulkanDescriptorPoolManager();

    
    // 分配描述符集
    VulkanDescriptorSets AllocateDescriptorSet(const VulkanDescriptorSetLayoutMap& map);

    

    // 释放所有池
    void Destroy();

private:
    VulkanDevice* _device = nullptr;
    std::vector<VulkanDescriptorPool*> _pools;

    // 查找合适的描述符池
    VulkanDescriptorPool* FindSuitablePool(const VulkanDescriptorSetLayoutMap& map);

    // 创建一个新的描述符池
    VulkanDescriptorPool* CreatePool(const VulkanDescriptorSetLayoutMap& map);
};


// 描述符集写入器（辅助更新）
class VulkanDescriptorSetWriter
{
public:
    VulkanDescriptorSetWriter();

    void WriteImage(VulkanDescriptorSet& descriptorSet, uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    void WriteBuffer(VulkanDescriptorSet& descriptorSet, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    void Clear() { _writes.clear(); }
    void Update(VkDevice device);

private:
    std::vector<VkWriteDescriptorSet> _writes;
};

} // namespace WR::RHIVulkan
