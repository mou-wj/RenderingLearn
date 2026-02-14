#include "VulkanShader.h"
#include <spirv_cross/spirv_cross.hpp>

namespace RHIVulkan {

VulkanRHIShader::VulkanRHIShader(VulkanDevice* device)
    : Device(device)
{
}

VulkanRHIShader::~VulkanRHIShader()
{
    Cleanup();
}

bool VulkanRHIShader::Initialize(const std::vector<char>& spirvCode)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(spirvCode.data());

    VkResult result = vkCreateShaderModule(Device->GetHandle(), &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        // Handle error
        return false;
    }

    return true;
}

void VulkanRHIShader::Cleanup()
{
    if (shaderModule != VK_NULL_HANDLE && Device)
    {
        vkDestroyShaderModule(Device->GetHandle(), shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }

}


} // namespace WR::RHIVulkan