#include "VulkanDescriptorSets.h"
#include <stdexcept>

namespace RHIVulkan {

// VulkanDescriptorSet 实现
VulkanDescriptorSet::VulkanDescriptorSet(VkDescriptorSet* set, VkDescriptorSetLayout layout, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    : bindings(bindings), _set(*set), _layout(layout) {}

// VulkanDescriptorPool 实现
VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
    : _device(device) {
    for (const auto& poolsize : poolSizes)
    {
		_validPoolSizes[poolsize.type] += poolsize.descriptorCount;
    }


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;

    if (vkCreateDescriptorPool(device->GetDevice(), &poolInfo, nullptr, &_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    vkDestroyDescriptorPool(_device->GetDevice(), _pool, nullptr);
    for (auto& descriptorSet : _allocatedSets) {
        delete descriptorSet;
    }
}

VulkanDescriptorSets VulkanDescriptorPool::AllocateDescriptorSet(const VulkanDescriptorSetLayoutMap& map) {
    std::vector<VkDescriptorSetLayout> layouts;
    std::vector<VkDescriptorSet> sets(map.size());


    for (const auto& pair : map) {
        layouts.push_back(CreateLayout(pair.second));
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(_device->GetDevice(), &allocInfo, sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VulkanDescriptorSets result;
    for (size_t i = 0; i < sets.size(); ++i) {
        result.push_back(new VulkanDescriptorSet(&sets[i], layouts[i], map.at(i)));
    }

    _allocatedSets.insert(_allocatedSets.end(), result.begin(), result.end());
    return result;
}

void VulkanDescriptorPool::Reset() {
    vkResetDescriptorPool(_device->GetDevice(), _pool, 0);
    for (auto& descriptorSet : _allocatedSets) {
        delete descriptorSet;
    }
    _allocatedSets.clear();
}

bool VulkanDescriptorPool::CanAllocate(const std::map<VkDescriptorType, uint32_t>& want) const {
	for (const auto& pair : want) {
		auto it = _validPoolSizes.find(pair.first);
		if (it == _validPoolSizes.end() || it->second < pair.second) {
			return false; // 不足以满足需求
		}
	}
	return true; // 可以满足所有需求
}

VkDescriptorSetLayout VulkanDescriptorPool::CreateLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VkDescriptorSetLayout _layout = VK_NULL_HANDLE;
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	if (vkCreateDescriptorSetLayout(_device->GetDevice(), &layoutInfo, nullptr, &_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}


    return _layout;
}

// VulkanDescriptorPoolManager 实现
VulkanDescriptorPoolManager::VulkanDescriptorPoolManager(VulkanDevice* device)
    : _device(device) {}

VulkanDescriptorPoolManager::~VulkanDescriptorPoolManager() {
    Destroy();
}

VulkanDescriptorSets VulkanDescriptorPoolManager::AllocateDescriptorSet(const VulkanDescriptorSetLayoutMap& map) {
    VulkanDescriptorPool* suitablePool = FindSuitablePool(map);
    if (suitablePool) {
        return suitablePool->AllocateDescriptorSet(map);
    } else {
        VulkanDescriptorPool* newPool = CreatePool(map);
        _pools.push_back(newPool);
        return newPool->AllocateDescriptorSet(map);
    }
}

void VulkanDescriptorPoolManager::Destroy() {
    for (auto& pool : _pools) {
        delete pool;
    }
    _pools.clear();
}

VulkanDescriptorPool* VulkanDescriptorPoolManager::FindSuitablePool(const VulkanDescriptorSetLayoutMap& map) {
	std::map<VkDescriptorType, uint32_t> totalPoolSizes;
    // 计算所需的描述符类型和数量
    for (const auto& pair : map) {
        for (const auto& binding : pair.second) {
            totalPoolSizes[binding.descriptorType] += binding.descriptorCount;
        }
    }
    
    for (auto& pool : _pools) {
		// 检查当前池是否包含所需的描述符类型
        
        // 这里假设每个池只有一种布局，可以根据需要进行扩展
        if (pool->CanAllocate(totalPoolSizes)) {
            return pool;
        }
    }
    
    return CreatePool(map); // 如果没有合适的池，则创建一个新的池
}

VulkanDescriptorPool* VulkanDescriptorPoolManager::CreatePool(const VulkanDescriptorSetLayoutMap& map) {
	std::map<VkDescriptorType, uint32_t> totalPoolSizes;
	// 计算所需的描述符类型和数量
	for (const auto& pair : map) {
		for (const auto& binding : pair.second) {
			totalPoolSizes[binding.descriptorType] += binding.descriptorCount;
		}
	}
	// 创建描述符池大小
    std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& pair : totalPoolSizes) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = pair.first;
		poolSize.descriptorCount = pair.second;
		poolSizes.push_back(poolSize);
	}

    return new VulkanDescriptorPool(_device, poolSizes, 30); // 创建一个包含10个描述符集的池
}

// VulkanDescriptorSetWriter 实现
VulkanDescriptorSetWriter::VulkanDescriptorSetWriter() {}

void VulkanDescriptorSetWriter::WriteImage(VulkanDescriptorSet& descriptorSet, uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type) {
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet.GetHandle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = imageInfo;

    _writes.push_back(descriptorWrite);
}

void VulkanDescriptorSetWriter::WriteBuffer(VulkanDescriptorSet& descriptorSet, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type) {
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet.GetHandle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = bufferInfo;

    _writes.push_back(descriptorWrite);
}

void VulkanDescriptorSetWriter::Update(VkDevice device) {
    if (!_writes.empty()) {
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
        Clear();
    }
}

} // namespace WR::RHIVulkan
