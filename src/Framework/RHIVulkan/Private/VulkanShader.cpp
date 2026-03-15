#include "VulkanShader.h"
#include "VulkanRHIUtils.h"
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

bool VulkanRHIShader::Initialize(const std::vector<char>& packedCode)
{
	RenderCore::SPIRVCompiledBinaryResultPacker packer;
	packer.Depack(packedCode);
	Reflection = packer.DepackedData.header;
    EntryPoint = packer.DepackedData.header.EntryPoint;
    auto spirvCode = packer.DepackedData.SpirvCode;
    ShaderStage = TransformShaderStageFrom(packer.DepackedData.header.Frequency);
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    createInfo.pCode = spirvCode.data();

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