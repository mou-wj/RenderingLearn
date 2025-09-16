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

    VkResult result = vkCreateShaderModule(Device->GetDevice(), &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        // Handle error
        return false;
    }

    ParseShaderDescriptorLayout(spirvCode);
    return true;
}

void VulkanRHIShader::Cleanup()
{
    if (shaderModule != VK_NULL_HANDLE && Device)
    {
        vkDestroyShaderModule(Device->GetDevice(), shaderModule, nullptr);
        shaderModule = VK_NULL_HANDLE;
    }

    DescriptorSetLayouts.clear();
}

void VulkanRHIShader::ParseShaderDescriptorLayout(const std::vector<char>& spirvCode)
{
    // Parse SPIR-V code, get descriptor set information
    spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(spirvCode.data()), spirvCode.size() / sizeof(uint32_t));
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    
	uint32_t setId = 0;
	uint32_t binding = 0;
	VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
	uint32_t desctiptorCount = 0;
	VkDescriptorSetLayoutBinding bindingInfo{};

	auto pushResBinding = [&](const spirv_cross::Resource& resVec, VkDescriptorType descriptorType) {
		
		auto type = compiler.get_type(resVec.base_type_id);
		setId = compiler.get_decoration(resVec.id, spv::Decoration::DecorationDescriptorSet);
		binding = compiler.get_decoration(resVec.id, spv::DecorationBinding);
		if (type.array.empty())
		{
			desctiptorCount = 1;
		}
		else {
			desctiptorCount = type.array[0];
		}
		bindingInfo.binding = binding;
		bindingInfo.descriptorType = descriptorType;
		bindingInfo.descriptorCount = desctiptorCount;
		DescriptorSetLayouts[setId].Bindings.push_back(bindingInfo);
		};

	for (uint32_t i = 0; i < resources.uniform_buffers.size(); i++)
	{
		auto& uniBuff = resources.uniform_buffers[i];
		pushResBinding(uniBuff, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	}


	for (uint32_t i = 0; i < resources.storage_buffers.size(); i++)
	{
		auto& storeBuffer = resources.storage_buffers[i];
		pushResBinding(storeBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}


	//
	for (uint32_t i = 0; i < resources.sampled_images.size(); i++)
	{
		auto& samplerImage = resources.sampled_images[i];
		pushResBinding(samplerImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	}


	
	for (uint32_t i = 0; i < resources.storage_images.size(); i++)
	{
		auto& storeImage = resources.storage_images[i];
		pushResBinding(storeImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	}

	
	for (uint32_t i = 0; i < resources.subpass_inputs.size(); i++)
	{
		auto& inputAttachment = resources.subpass_inputs[i];
		pushResBinding(inputAttachment, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

	}


	//acceleration struct
	for (uint32_t i = 0; i < resources.acceleration_structures.size(); i++)
	{
		auto& acceleration = resources.acceleration_structures[i];
		pushResBinding(acceleration, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);


	}



}

} // namespace WR::RHIVulkan