#pragma once

#include "VulkanDevice.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "VulkanDescriptorSets.h"
#include "VulkanRenderPass.h"
#include "VulkanResource.h"
#include "RHIDefine.h"
#include "VulkanFuncWrapper.h"
#include "HashHelper.hpp"

namespace RHIVulkan{
    struct PipelineLayoutInfo
    {
        // 按着色器频率分组管理 descriptor set layouts
        // key: ERHIShaderFrequency cast to uint32_t, value: descriptor set layouts for that stage
		struct ShaderFrequencyLayoutInfo
		{
			std::vector<DescriptorSetLayoutInfo> Layouts;
			int GlobalUniformBufferBinding = -1; // 全局 uniform buffer 的绑定点，-1 表示没有
            int GlobalUniformBufferSet = -1; // 全局 uniform buffer 的 set，-1 表示没有
		};


        std::unordered_map<uint32_t, ShaderFrequencyLayoutInfo> setLayoutsByFrequency;
        VkPushConstantRange pushConstant{};
        bool hasPushConstant = false;

        // 获取指定着色器频率的 descriptor set layouts
        const ShaderFrequencyLayoutInfo* GetLayoutsForFrequency(RHI::ERHIShaderFrequency frequency) const
        {
            auto it = setLayoutsByFrequency.find(static_cast<uint32_t>(frequency));
            if (it != setLayoutsByFrequency.end()) {
                return &it->second;
            }
            return nullptr;
        }

        // 为指定着色器频率添加 descriptor set layout
        void AddLayoutForFrequency(RHI::ERHIShaderFrequency frequency, const DescriptorSetLayoutInfo& layout, int globalBufferBinding = -1, int globalBufferSet = -1)
        {
            ShaderFrequencyLayoutInfo& freqInfo = setLayoutsByFrequency[static_cast<uint32_t>(frequency)];
            freqInfo.Layouts.push_back(layout);
            if (globalBufferBinding >= 0) {
                freqInfo.GlobalUniformBufferBinding = globalBufferBinding;
            }
            if (globalBufferSet >= 0) {
                freqInfo.GlobalUniformBufferSet = globalBufferSet;
            }
        }

        // 获取所有着色器频率的 descriptor set layouts（展平）
        std::vector<DescriptorSetLayoutInfo> GetAllLayouts() const
        {
            std::vector<DescriptorSetLayoutInfo> result;
            for (const auto& pair : setLayoutsByFrequency) {
                result.insert(result.end(), pair.second.Layouts.begin(), pair.second.Layouts.end());
            }
            return result;
        }

        static uint64_t CalculateHash(const PipelineLayoutInfo& info)
        {
            size_t h = 0;

            // Hash setLayoutsByFrequency - iterate through all frequencies
            for (const auto& frequencyPair : info.setLayoutsByFrequency)
            {
                // Hash the frequency
                HashCombine(h, std::hash<uint32_t>()(frequencyPair.first));

                const ShaderFrequencyLayoutInfo& freqInfo = frequencyPair.second;

                // Hash all layouts for this frequency
                for (const auto& layout : freqInfo.Layouts)
                {
                    HashCombine(h, std::hash<uint64_t>()((uint64_t)DescriptorSetLayoutInfo::CalculateHash(layout)));
                }

                // Hash global uniform buffer binding info
                if (freqInfo.GlobalUniformBufferBinding >= 0) {
                    HashCombine(h, std::hash<int>()(freqInfo.GlobalUniformBufferBinding));
                }
                if (freqInfo.GlobalUniformBufferSet >= 0) {
                    HashCombine(h, std::hash<int>()(freqInfo.GlobalUniformBufferSet));
                }
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

            // 获取所有着色器频率的 descriptor set layouts
            std::vector<DescriptorSetLayoutInfo> allLayouts = info.GetAllLayouts();
            createInfo.setLayoutCount = (uint32_t)allLayouts.size();

            std::vector<VkDescriptorSetLayout> layouts;
            for (const auto& layout : allLayouts)
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

            VKFunc::CreatePipelineLayout(
                device->GetHandle(),
                &createInfo,
                &layout);
        }

        ~VulkanPipelineLayout()
        {
            if (layout != VK_NULL_HANDLE) 
            {
                VKFunc::DestroyPipelineLayout(device->GetHandle(), layout);
            }
                
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
        ~VulkanPipelineLayoutCache() 
        {
			Layouts.clear();
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
class VulkanComputePipelineState : public VulkanPipelineBase ,public RHIComputePipelineState {
public:
    VulkanComputePipelineState(VulkanDevice* device, const RHIComputePipelineStateDesc& pipelineDesc);
    ~VulkanComputePipelineState();
    void Bind(VulkanCommandBuffer* cmdBuffer);

private:
    PipelineLayoutInfo BuildPipelineLayoutInfo(const RHIComputePipelineStateDesc& pipelineDesc);
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
using VulkanComputePipelineStateSP = std::shared_ptr<VulkanComputePipelineState>;
using VulkanRayTracingPipelineSP = std::shared_ptr<VulkanRayTracingPipeline>;

}
