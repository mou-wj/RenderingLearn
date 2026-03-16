#pragma once
#include "RHIDefine.h"
#include "ShaderCompiledDataPacker.h"
#include <vulkan/vulkan.h>
using namespace RHI;
namespace RHIVulkan {

    VkFormat TransformFormatFrom(ERHIFormat format);

    VkShaderStageFlagBits TransformShaderStageFrom(ERHIShaderFrequency shaderType) ;

    VkImageUsageFlags TransformTextureUsageFlagsFrom(ERHITextureCreateFlags Flags);

	VkSampleCountFlagBits TransformSampleCountFrom(uint32_t sampleCount);

    VkImageType TransformImageTypeFrom(ERHITextureType type);

    VkImageViewType TransformViewTypeFrom(ERHITextureType type);

    VkImageAspectFlags GetImageAspectFlags(VkFormat format);

    VkDescriptorType TransformDescriptorTypeFrom(RenderCore::SPIRVCompiledBinaryResultPacker::ESPIRVShaderResourceType type);

	VkShaderStageFlagBits TransformShaderFrequencyToStage(ERHIShaderFrequency frequency);

    VkPrimitiveTopology TransformPrimitiveTopology(EPrimitiveTopology Topology);

    VkBufferUsageFlags TransformBufferUsageFlagsFrom(ERHIBufferUsageFlags Flags);

}