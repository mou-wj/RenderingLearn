#include "VulkanShader.h"
#include "VulkanRHIUtils.h"
#include "VulkanFuncWrapper.h"
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

    bool result = VKFunc::CreateShaderModule(Device->GetHandle(), &createInfo,  &shaderModule);
    return result;
}

void VulkanRHIShader::Cleanup()
{
    if (shaderModule != VK_NULL_HANDLE && Device)
    {
        VKFunc::DestroyShaderModule(Device->GetHandle(), shaderModule);
        shaderModule = VK_NULL_HANDLE;
    }

}


} // namespace WR::RHIVulkan