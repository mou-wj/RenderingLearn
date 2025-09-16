#pragma once
#include "RHIDefine.h"
#include <vulkan/vulkan.h>
using namespace RHI;
namespace RHIVulkan {

    VkFormat TransforFormatFrom(ERHIFormat format);

    VkShaderStageFlagBits TransforShaderStageFrom(ERHIShaderType shaderType) ;



}