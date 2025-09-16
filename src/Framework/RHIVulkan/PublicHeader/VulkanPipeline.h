#pragma once

#include "VulkanDevice.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "RHIResource.h"
#include "VulkanDescriptorSets.h"
#include "VulkanRenderPass.h"
using namespace RHI;

namespace RHIVulkan{

// Pipeline基类
class VulkanPipelineBase {
public:
    explicit VulkanPipelineBase(VulkanDevice* device)
        : device(device), pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE) {}
    virtual ~VulkanPipelineBase();

    VkPipeline GetHandle() const { return pipeline; }
    VkPipelineLayout GetLayout() const { return pipelineLayout; }

protected:
    void AllocateDescriptorSets(const std::vector<RHIShaderStageDesc>& shaderStages);
    VulkanDevice* device;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VulkanDescriptorSets boundDescriptorSets; // 绑定的描述符集
    VulkanRenderPass* renderPass;
    
    void CreateLayout(const std::vector<RHIShaderStageDesc>& shaderStages);
    virtual void CreatePipeline() = 0;
};

// 图形管线
class VulkanGraphicsPipeline : public VulkanPipelineBase ,public RHIGraphicsPipelineState {
public:
    VulkanGraphicsPipeline(VulkanDevice* device, const RHIGraphicsPipelineStateDesc& pipelineDesc);
    ~VulkanGraphicsPipeline();

private:
    VkGraphicsPipelineCreateInfo createInfo; // Vulkan-specific create info

    void CreatePipeline() override;
    std::vector<VkDynamicState>  dynamicStates; // 动态状态列表
};

// 计算管线
class VulkanComputePipeline : public VulkanPipelineBase ,public RHIComputePipelineState {
public:
    VulkanComputePipeline(VulkanDevice* device, const RHIComputePipelineStateDesc& pipelineDesc);
    ~VulkanComputePipeline();

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
using VulkanGraphicsPipelineSP = std::shared_ptr<VulkanGraphicsPipeline>;
using VulkanComputePipelineSP = std::shared_ptr<VulkanComputePipeline>;
using VulkanRayTracingPipelineSP = std::shared_ptr<VulkanRayTracingPipeline>;


}
