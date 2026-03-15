#pragma once

#include "VulkanDevice.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "VulkanDescriptorSets.h"
#include "VulkanRenderPass.h"
#include "VulkanResource.h"
inline void HashCombine(size_t& seed, size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace RHIVulkan{
    struct PipelineLayoutInfo
    {
        std::vector<DescriptorSetLayoutInfo> setLayouts;

        VkPushConstantRange pushConstant{};
        bool hasPushConstant = false;

        static uint64_t CalculateHash(const PipelineLayoutInfo& info)
        {
            size_t h = 0;

            for (auto layout : info.setLayouts)
            {
                HashCombine(h, std::hash<uint64_t>()((uint64_t)DescriptorSetLayoutInfo::CalculateHash(layout)));
            }

            if (info.hasPushConstant)
            {
                HashCombine(h, info.pushConstant.offset);
                HashCombine(h, info.pushConstant.size);
                HashCombine(h, info.pushConstant.stageFlags);
            }

            return h;
        }
    };

    class VulkanPipelineLayout
    {
    public:

        VulkanPipelineLayout(
            VulkanDevice* Device,
            const PipelineLayoutInfo& info)
            : device(Device)
            , info(info)
        {
            VkPipelineLayoutCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

            createInfo.setLayoutCount =
                (uint32_t)info.setLayouts.size();

            std::vector<VkDescriptorSetLayout> layouts;
            for (auto layout : info.setLayouts)
			{
				layouts.push_back(
					device->GetDescriptorSetLayoutManager()->GetOrCreateLayout(layout));
			}
            createInfo.pSetLayouts =
                layouts.data();

            if (info.hasPushConstant)
            {
                createInfo.pushConstantRangeCount = 1;
                createInfo.pPushConstantRanges = &info.pushConstant;
            }

            vkCreatePipelineLayout(
                device->GetHandle(),
                &createInfo,
                nullptr,
                &layout);
        }

        ~VulkanPipelineLayout()
        {
            if (layout != VK_NULL_HANDLE)
                vkDestroyPipelineLayout(device->GetHandle(), layout, nullptr);
        }

        VkPipelineLayout GetHandle()
        {
            return layout;
        }
        const PipelineLayoutInfo& GetInfo() const { return info; }

    private:

        VulkanDevice* device;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        PipelineLayoutInfo info;
        friend class VulkanPipelineStateCache;
    };

    class VulkanPipelineLayoutCache
    {
    public:

        VulkanPipelineLayoutCache(VulkanDevice* device)
            : device(device)
        {
        }

        VulkanPipelineLayout* GetOrCreateLayout(
            const PipelineLayoutInfo& info)
        {
            uint64_t hash = PipelineLayoutInfo::CalculateHash(info);

            auto it = Layouts.find(hash);
            if (it != Layouts.end())
            {
                return it->second.get();
            }

            auto layout =
                std::make_unique<VulkanPipelineLayout>(
                    device,
                    info);

            VulkanPipelineLayout* result = layout.get();

            Layouts[hash] = std::move(layout);

            return result;
        }

    private:

        VulkanDevice* device;

        std::unordered_map<
            uint64_t,
            std::unique_ptr<VulkanPipelineLayout>> Layouts;
    };



// Pipeline基类
class VulkanPipelineBase {
public:
    explicit VulkanPipelineBase(VulkanDevice* device);
    virtual ~VulkanPipelineBase();

    VkPipeline GetHandle() const { return pipeline; }
    VulkanPipelineLayout* GetLayout() const { return pipelineLayout; }

protected:

    VulkanDevice* device;
    VkPipeline pipeline;
	VulkanPipelineLayout* pipelineLayout;
    //VulkanDescriptorSets boundDescriptorSets; // 绑定的描述符集
    VulkanRenderPass* renderPass;
    friend class VulkanPipelineStateCache;
    
    virtual void CreatePipeline() = 0;
};

// 图形管线
class VulkanGraphicsPipelineState : public VulkanPipelineBase ,public RHIGraphicsPipelineState {
public:
    VulkanGraphicsPipelineState(VulkanDevice* device, const RHIGraphicsPipelineStateDesc& pipelineDesc);
    ~VulkanGraphicsPipelineState();
    void Bind(VulkanCommandBuffer* cmdBuffer);

private:
    PipelineLayoutInfo BuildPipelineLayoutInfo(
        const RHIGraphicsPipelineStateDesc& pipelineDesc);
    VkGraphicsPipelineCreateInfo createInfo; // Vulkan-specific create info

    void CreatePipeline() override;
    std::vector<VkDynamicState>  dynamicStates; // 动态状态列表
};

// 计算管线
class VulkanComputePipeline : public VulkanPipelineBase ,public RHIComputePipelineState {
public:
    VulkanComputePipeline(VulkanDevice* device, const RHIComputePipelineStateDesc& pipelineDesc);
    ~VulkanComputePipeline();
    void Bind(VulkanCommandBuffer* cmdBuffer);

private:
    VkComputePipelineCreateInfo createInfo; // Vulkan-specific create info

    void CreatePipeline() override;
};

// 光线追踪管线
class VulkanRayTracingPipeline : public VulkanPipelineBase,public RHIRayTracingPipelineState {
public:
    VulkanRayTracingPipeline(VulkanDevice* device, const RHIRayTracingPipelineStateDesc& pipelineDesc);
    ~VulkanRayTracingPipeline();

private:
    VkRayTracingPipelineCreateInfoKHR createInfo; // Vulkan-specific create info

    void CreatePipeline() override;
};

using VulkanPipelineSP = std::shared_ptr<VulkanPipelineBase>;
using VulkanGraphicsPipelineStateSP = std::shared_ptr<VulkanGraphicsPipelineState>;
using VulkanComputePipelineSP = std::shared_ptr<VulkanComputePipeline>;
using VulkanRayTracingPipelineSP = std::shared_ptr<VulkanRayTracingPipeline>;

}
