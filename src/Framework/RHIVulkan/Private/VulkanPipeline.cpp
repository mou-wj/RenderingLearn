#include "VulkanPipeline.h"
#include "VulkanShader.h"

#include "VulkanDescriptorSets.h"
#include "VulkanResource.h"
#include "VulkanRHIUtils.h"
#include <vector>

using namespace RHI;

namespace RHIVulkan {

VulkanPipelineBase::~VulkanPipelineBase() {
    VkDevice deviceHandle = device->GetDevice();
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(deviceHandle, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(deviceHandle, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }

    // 释放所有描述符集
    for (auto& descriptorSet : boundDescriptorSets) {
        delete descriptorSet;
    }
    boundDescriptorSets.clear();
}



void VulkanPipelineBase::CreateLayout(const std::vector<RHIShaderStageDesc>& shaderStages) {
    // 获取描述符集布局信息
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutHandles;
    VulkanDescriptorSetLayoutMap layoutMap;
    std::vector<int> l;
    
    for (auto& shader : shaderStages)
    {
        auto vulkanShaderSP = std::reinterpret_pointer_cast<VulkanRHIShader>(shader.shader);
		const auto& descSetLayouts = vulkanShaderSP->GetDescriptorSetLayouts();
        
        for (const auto& setBindings : descSetLayouts)
        {
            auto setId = setBindings.first;
            auto& bindings = setBindings.second.Bindings;
            layoutMap[setId].insert(layoutMap[setId].end(), bindings.begin(), bindings.end());
        }


        

    }


    // 分配描述符集
    boundDescriptorSets = device->GetDescriptorPoolManager()->AllocateDescriptorSet(layoutMap);

    // 创建VkPipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(boundDescriptorSets.size());
    std::vector<VkDescriptorSetLayout> setLayoutHandles;
    for (auto& descriptorSet : boundDescriptorSets) {
        setLayoutHandles.push_back(descriptorSet->GetLayout());
    }
    pipelineLayoutInfo.pSetLayouts = setLayoutHandles.data();

    VkResult result = vkCreatePipelineLayout(device->GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("无法创建管道布局!");
    }
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, const RHIGraphicsPipelineStateDesc& pipelineDesc)
    : VulkanPipelineBase(device), RHIGraphicsPipelineState(pipelineDesc) {
    // Convert RHI desc to Vulkan create info
    createInfo = {}; // Initialize createInfo
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.renderPass = renderPass->GetVkRenderPass();

    CreateLayout(pipelineDesc.shaderStages);
    CreatePipeline();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
}


void VulkanGraphicsPipeline::CreatePipeline() {
    // 设置VkGraphicsPipelineCreateInfo的其他成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充
    VkPipelineVertexInputStateCreateInfo &vertexInputInfo = std::static_pointer_cast<VulkanRHIVertexDescState>(desc.vertexDescState)->vertexInputInfo;

    VkPipelineRasterizationStateCreateInfo rasterizer = std::static_pointer_cast<VulkanRHIRasterizerState>(desc.rasterizerState)->rasterizerInfo;

    VkPipelineColorBlendStateCreateInfo colorBlending = std::static_pointer_cast<VulkanRHIColorBlendState>(desc.colorBlendState)->colorBlendInfo;

    VkPipelineDepthStencilStateCreateInfo depthStencil = std::static_pointer_cast<VulkanRHIDepthStencilState>(desc.depthStencilState)->depthStencilInfo;

    // 设置管线布局
    createInfo.layout = pipelineLayout;


    
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    dynamicStates.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

        // 设置createInfo的各个成员
    createInfo.pVertexInputState = &vertexInputInfo;
    createInfo.pInputAssemblyState = nullptr;
    createInfo.pViewportState = nullptr;
    createInfo.pColorBlendState = &colorBlending;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = nullptr;
    createInfo.pDynamicState = &dynamicState;



    // 设置着色器阶段
    createInfo.stageCount = static_cast<uint32_t>(desc.shaderStages.size());
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(createInfo.stageCount);

    for (size_t i = 0; i < desc.shaderStages.size(); ++i) {
        const auto& stage = desc.shaderStages[i];
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        auto vulkanShaderSP = std::reinterpret_pointer_cast<VulkanRHIShader>(stage.shader);
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = TransforShaderStageFrom(stage.shader->GetShaderType());
        shaderStageInfo.module = vulkanShaderSP->GetShaderModule();
        shaderStageInfo.pName = vulkanShaderSP->GetEntryPoint().c_str();
        shaderStages[i] = shaderStageInfo;
    }

    createInfo.pStages = shaderStages.data();

    // 创建图形管线
    VkResult result = vkCreateGraphicsPipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("无法创建图形管线!");
    }
}

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device, const RHIComputePipelineStateDesc& pipelineDesc)
    : VulkanPipelineBase(device), RHIComputePipelineState(pipelineDesc) {
    // Convert RHI desc to Vulkan create info
    createInfo = {}; // Initialize createInfo
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.layout = VK_NULL_HANDLE;

    CreateLayout({ pipelineDesc.shaderDesc});
    CreatePipeline();
}

VulkanComputePipeline::~VulkanComputePipeline() {
}

void VulkanComputePipeline::CreatePipeline() {
    // 设置VkComputePipelineCreateInfo的各个成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充

    // 设置着色器阶段
    auto vulkanShaderSP = std::reinterpret_pointer_cast<VulkanRHIShader>(desc.shaderDesc.shader);
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = vulkanShaderSP->GetShaderStage(); // 计算着色器阶段
    shaderStageInfo.module = vulkanShaderSP->GetShaderModule();
    shaderStageInfo.pName = vulkanShaderSP->GetEntryPoint().c_str();


    // 创建计算管线
    VkResult result = vkCreateComputePipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("无法创建计算管线!");
    }
}

VulkanRayTracingPipeline::VulkanRayTracingPipeline(VulkanDevice* device, const RHIRayTracingPipelineStateDesc& pipelineDesc)
    : VulkanPipelineBase(device), RHIRayTracingPipelineState(pipelineDesc) {
    // Convert RHI desc to Vulkan create info
    createInfo = {}; // Initialize createInfo
    createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    createInfo.layout = VK_NULL_HANDLE;

    CreateLayout({});
    CreatePipeline();
}

VulkanRayTracingPipeline::~VulkanRayTracingPipeline() {
}

void VulkanRayTracingPipeline::CreatePipeline() {
    // 设置VkRayTracingPipelineCreateInfoKHR的各个成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充

    // 设置着色器阶段
    //createInfo.stageCount = static_cast<uint32_t>(pipelineDesc.ShaderStages.size());
    //std::vector<VkPipelineShaderStageCreateInfo> shaderStages(createInfo.stageCount);

    //for (size_t i = 0; i < pipelineDesc.ShaderStages.size(); ++i) {
    //    const auto& stage = pipelineDesc.ShaderStages[i];
    //    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    //    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //    shaderStageInfo.stage = stage.ShaderStage;
    //    shaderStageInfo.module = stage.ShaderModule->GetHandle();
    //    shaderStageInfo.pName = stage.EntryPoint.c_str();
    //    shaderStages[i] = shaderStageInfo;
    //}

    //createInfo.pStages = shaderStages.data();

    //// 设置光线追踪相关的其他成员
    //// 这里仅提供一个示例，具体成员需要根据pipelineDesc填充

    //createInfo.pGroups = pipelineDesc.pGroups;
    //createInfo.groupCount = pipelineDesc.groupCount;
    //createInfo.maxRecursionDepth = pipelineDesc.maxRecursionDepth;

    //// 创建光线追踪管线
    //VkResult result = vkCreateRayTracingPipelinesKHR(device->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
    //if (result != VK_SUCCESS) {
    //    throw std::runtime_error("无法创建光线追踪管线!");
    //}
}

} // namespace WR::RHIVulkan
