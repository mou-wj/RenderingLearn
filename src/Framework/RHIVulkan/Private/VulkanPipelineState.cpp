#include "VulkanPipelineState.h"
#include "VulkanShader.h"

#include "VulkanDescriptorSets.h"
#include "VulkanResource.h"
#include "VulkanRHIUtils.h"
#include "VulkanCommandBuffer.h"
#include <vector>

using namespace RHI;

namespace RHIVulkan {

VulkanPipelineBase::VulkanPipelineBase(VulkanDevice* device)
: device(device), pipeline(VK_NULL_HANDLE)
{

}
VulkanPipelineBase::~VulkanPipelineBase() {
    VkDevice deviceHandle = device->GetHandle();
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(deviceHandle, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }

}



VulkanGraphicsPipelineState::VulkanGraphicsPipelineState(VulkanDevice* device, const RHIGraphicsPipelineStateDesc& pipelineDesc)
    : VulkanPipelineBase(device), RHIGraphicsPipelineState(pipelineDesc) {
    // Convert RHI desc to Vulkan create info
    createInfo = {}; // Initialize createInfo
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    VulkanRenderTargetLayout layout(pipelineDesc.attachmentDesc);
    auto renderPass = device->GetRenderPassManager()->GetOrCreateRenderPass(layout);
    createInfo.renderPass = renderPass->GetHandle();
    createInfo.subpass = pipelineDesc.attachmentDesc.subpassIndex;
	auto layoutInfo = BuildPipelineLayoutInfo(pipelineDesc);
    pipelineLayout = device->GetPipelineLayoutCache()->GetOrCreateLayout(layoutInfo);
    
    CreatePipeline();
}

VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState() {
}

void VulkanGraphicsPipelineState::Bind(VulkanCommandBuffer* cmdBuffer)
{
    vkCmdBindPipeline(cmdBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

PipelineLayoutInfo VulkanGraphicsPipelineState::BuildPipelineLayoutInfo(const RHIGraphicsPipelineStateDesc& pipelineDesc)
{
    PipelineLayoutInfo layoutInfo;

    auto processShader = [&](VulkanRHIShader* shader)
        {
            if (!shader)
                return;

            const auto& header = shader->GetShaderReflection();

            VkShaderStageFlags stageMask =
                TransformShaderFrequencyToStage(header.Frequency);

            auto ensureSet = [&](uint32_t set)
                {
                    if (layoutInfo.setLayouts.size() <= set)
                    {
                        layoutInfo.setLayouts.resize(set + 1);
                    }
                };

            for (const auto& binding : header.DescriptorBindings)
            {
                ensureSet(binding.Set);

                DescriptorSetLayoutInfo& setLayout =
                    layoutInfo.setLayouts[binding.Set];

                VkDescriptorType vkType = TransformDescriptorTypeFrom(binding.Type);

                setLayout.AddBinding(
                    binding.Binding,
                    vkType,
                    binding.Count,
                    stageMask);
            }

            if (header.HasPushConstant)
            {
                layoutInfo.hasPushConstant = true;

                layoutInfo.pushConstant.offset = 0;
                layoutInfo.pushConstant.size = header.PushConstant.Size;

                layoutInfo.pushConstant.stageFlags |=
                    stageMask;
            }
        };

    processShader(
        dynamic_cast<VulkanRHIShader*>(pipelineDesc.shaderStages.vertexShader));

    processShader(
        dynamic_cast<VulkanRHIShader*>(pipelineDesc.shaderStages.fragmentShader));

    return layoutInfo;
}


void VulkanGraphicsPipelineState::CreatePipeline() {
    // 设置VkGraphicsPipelineCreateInfo的其他成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充
    VkPipelineVertexInputStateCreateInfo &vertexInputInfo = dynamic_cast<VulkanVertexDescState*>(desc.vertexDescState)->vertexInputInfo;

    VkPipelineRasterizationStateCreateInfo rasterizer = dynamic_cast<VulkanRasterizerState*>(desc.rasterizerState)->rasterizerInfo;

    VkPipelineColorBlendStateCreateInfo colorBlending = dynamic_cast<VulkanColorBlendState*>(desc.colorBlendState)->colorBlendInfo;

    VkPipelineDepthStencilStateCreateInfo depthStencil = dynamic_cast<VulkanDepthStencilState*>(desc.depthStencilState)->depthStencilInfo;

    // 设置管线布局
    createInfo.layout = pipelineLayout->GetHandle();


    
    dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = TransformPrimitiveTopology(desc.primitiveTopology);
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;
    viewportState.pViewports = nullptr;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = TransformSampleCountFrom(desc.attachmentDesc.numSamples);
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 1.0f;
    multisample.pSampleMask = nullptr;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;


        // 设置createInfo的各个成员
    createInfo.pVertexInputState = &vertexInputInfo;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pViewportState = &viewportState;
    createInfo.pColorBlendState = &colorBlending;
    createInfo.pDepthStencilState = &depthStencil;
    createInfo.pRasterizationState = &rasterizer;
    createInfo.pMultisampleState = &multisample;
    createInfo.pDynamicState = &dynamicState;



    // 设置着色器阶段
	std::vector<RHI::RHIShader*> graphicShaders;
    if (desc.shaderStages.vertexShader != nullptr) {
        graphicShaders.push_back(desc.shaderStages.vertexShader);
    }
	if (desc.shaderStages.fragmentShader != nullptr) {
		graphicShaders.push_back(desc.shaderStages.fragmentShader);
	}

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(graphicShaders.size());

    for (size_t i = 0; i < graphicShaders.size(); ++i) {
        const auto& shader = graphicShaders[i];
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        auto vulkanShaderSP = dynamic_cast<VulkanRHIShader*>(shader);
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = vulkanShaderSP->GetShaderStage();
        shaderStageInfo.module = vulkanShaderSP->GetShaderModule();
        shaderStageInfo.pName = vulkanShaderSP->GetEntryPoint().c_str();
        shaderStages[i] = shaderStageInfo;
    }
    createInfo.stageCount = shaderStages.size();
    createInfo.pStages = shaderStages.data();

    // 创建图形管线
    VkResult result = vkCreateGraphicsPipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
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

    CreatePipeline();
}

VulkanComputePipeline::~VulkanComputePipeline() {
}

void VulkanComputePipeline::Bind(VulkanCommandBuffer* cmdBuffer)
{
    vkCmdBindPipeline(cmdBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void VulkanComputePipeline::CreatePipeline() {
    // 设置VkComputePipelineCreateInfo的各个成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充

    // 设置着色器阶段
    auto vulkanShaderSP = reinterpret_cast<VulkanRHIShader*>(desc.computeShader);
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = vulkanShaderSP->GetShaderStage(); // 计算着色器阶段
    shaderStageInfo.module = vulkanShaderSP->GetShaderModule();
    shaderStageInfo.pName = vulkanShaderSP->GetEntryPoint().c_str();


    // 创建计算管线
    VkResult result = vkCreateComputePipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
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
    //VkResult result = vkCreateRayTracingPipelinesKHR(device->GetHandle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
    //if (result != VK_SUCCESS) {
    //    throw std::runtime_error("无法创建光线追踪管线!");
    //}
}

} // namespace WR::RHIVulkan
