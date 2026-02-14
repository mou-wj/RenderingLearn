#pragma once
#include "RHIDefine.h"
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

}