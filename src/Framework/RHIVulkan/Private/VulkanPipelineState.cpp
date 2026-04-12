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
        device->EnqueuePipelineForDeletion(pipeline);
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
    VKFunc::CmdBindPipeline(cmdBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

PipelineLayoutInfo VulkanGraphicsPipelineState::BuildPipelineLayoutInfo(const RHIGraphicsPipelineStateDesc& pipelineDesc)
{
    PipelineLayoutInfo layoutInfo;

    auto processShader = [&](VulkanRHIShader* shader)
        {
            if (!shader)
                return;

            const auto& header = shader->GetShaderReflection();

            RHI::ERHIShaderFrequency frequency = header.Frequency;
            VkShaderStageFlags stageMask =
                TransformShaderFrequencyToStage(frequency);

            // 获取或创建该着色器频率对应的 ShaderFrequencyLayoutInfo
            uint32_t freqKey = static_cast<uint32_t>(frequency);
            PipelineLayoutInfo::ShaderFrequencyLayoutInfo& freqInfo = 
                layoutInfo.setLayoutsByFrequency[freqKey];

            auto ensureSet = [&](uint32_t set)
                {
                    if (freqInfo.Layouts.size() <= set)
                    {
                        freqInfo.Layouts.resize(set + 1);
                    }
                };

            for (const auto& binding : header.DescriptorBindings)
            {
                ensureSet(binding.Set);

                DescriptorSetLayoutInfo& setLayout = freqInfo.Layouts[binding.Set];

                VkDescriptorType vkType = TransformDescriptorTypeFrom(binding.Type);

                setLayout.AddBinding(
                    binding.Binding,
                    vkType,
                    binding.Count,
                    stageMask);
            }
            
            if (header.GlobalUniformBufferSet != -1) {
				ensureSet(header.GlobalUniformBufferSet);
				freqInfo.GlobalUniformBufferSet = header.GlobalUniformBufferSet;
				freqInfo.GlobalUniformBufferBinding = header.GlobalUniformBufferBinding;
			
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
    if (!VKFunc::CreateGraphicsPipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, &pipeline)) {
        throw std::runtime_error("无法创建图形管线!");
    }
}


VulkanComputePipelineState::VulkanComputePipelineState(VulkanDevice* device, const RHIComputePipelineStateDesc& pipelineDesc)
    : VulkanPipelineBase(device), RHIComputePipelineState(pipelineDesc) {
    // Convert RHI desc to Vulkan create info
    createInfo = {}; // Initialize createInfo
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;

    // Build layout and cache it
    PipelineLayoutInfo layoutInfo = BuildPipelineLayoutInfo(pipelineDesc);
    pipelineLayout = device->GetPipelineLayoutCache()->GetOrCreateLayout(layoutInfo);
    createInfo.layout = pipelineLayout->GetHandle();

    CreatePipeline();
}

VulkanComputePipelineState::~VulkanComputePipelineState() {
}

void VulkanComputePipelineState::Bind(VulkanCommandBuffer* cmdBuffer)
{
    VKFunc::CmdBindPipeline(cmdBuffer->GetHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

PipelineLayoutInfo VulkanComputePipelineState::BuildPipelineLayoutInfo(const RHIComputePipelineStateDesc& pipelineDesc)
{
    PipelineLayoutInfo layoutInfo;

    auto vulkanShaderSP = dynamic_cast<VulkanRHIShader*>(pipelineDesc.computeShader);
    if (!vulkanShaderSP)
        return layoutInfo;

    const auto& header = vulkanShaderSP->GetShaderReflection();
    RHI::ERHIShaderFrequency frequency = header.Frequency;
    VkShaderStageFlags stageMask = TransformShaderFrequencyToStage(frequency);

    // 获取该着色器频率对应的 ShaderFrequencyLayoutInfo
    uint32_t freqKey = static_cast<uint32_t>(frequency);
    PipelineLayoutInfo::ShaderFrequencyLayoutInfo& freqInfo = 
        layoutInfo.setLayoutsByFrequency[freqKey];

    auto ensureSet = [&](uint32_t set)
        {
            if (freqInfo.Layouts.size() <= set)
            {
                freqInfo.Layouts.resize(set + 1);
            }
        };

    for (const auto& binding : header.DescriptorBindings)
    {
        ensureSet(binding.Set);

        DescriptorSetLayoutInfo& setLayout = freqInfo.Layouts[binding.Set];

        VkDescriptorType vkType = TransformDescriptorTypeFrom(binding.Type);

        setLayout.AddBinding(
            binding.Binding,
            vkType,
            binding.Count,
            stageMask);
    }

    if (header.GlobalUniformBufferSet != -1) {
        ensureSet(header.GlobalUniformBufferSet);
        freqInfo.GlobalUniformBufferSet = header.GlobalUniformBufferSet;
        freqInfo.GlobalUniformBufferBinding = header.GlobalUniformBufferBinding;
       
    }

    if (header.HasPushConstant)
    {
        layoutInfo.hasPushConstant = true;

        layoutInfo.pushConstant.offset = 0;
        layoutInfo.pushConstant.size = header.PushConstant.Size;

        layoutInfo.pushConstant.stageFlags = stageMask;
    }

    return layoutInfo;
}


void VulkanComputePipelineState::CreatePipeline() {
    // 设置VkComputePipelineCreateInfo的各个成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充

    // 设置着色器阶段
    auto vulkanShaderSP = dynamic_cast<VulkanRHIShader*>(desc.computeShader);
    if (!vulkanShaderSP)
        throw std::runtime_error("Compute shader is null for compute pipeline");

    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = vulkanShaderSP->GetShaderStage(); // 计算着色器阶段
    shaderStageInfo.module = vulkanShaderSP->GetShaderModule();
    shaderStageInfo.pName = vulkanShaderSP->GetEntryPoint().c_str();

    createInfo.stage = shaderStageInfo;

    // 创建计算管线
    if (!VKFunc::CreateComputePipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo, &pipeline)) {
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
