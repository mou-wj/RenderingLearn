#include "ExampleBase.h"

#include <SPIRV/GlslangToSpv.h>
#include <Public/ShaderLang.h>
#include <Include/ResourceLimits.h>
#include <Public/resource_limits_c.h>

#include <shaderc/shaderc.h>
#include <filesystem>
#include <fstream>

using namespace VulkanAPI;


void ExampleBase::Init()
{
	Initialize();
}

void ExampleBase::ParseShaderFiles(std::vector<ShaderCodePaths>& shaderPaths)
{
	pipelineShaderResourceInfos.resize(shaderPaths.size());
	std::vector<char> tmpCode;
	for (uint32_t i = 0; i < shaderPaths.size(); i++)
	{
		pipelineShaderResourceInfos[i].shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].shaderFilePath = shaderPaths[i].vertexShaderPath;
		ReadGLSLShaderFile(shaderPaths[i].vertexShaderPath, tmpCode);
		CompileGLSLToSPIRV(VK_SHADER_STAGE_VERTEX_BIT,tmpCode, pipelineShaderResourceInfos[i].shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode);
		//解析vertex attribute info
		//spirv_cross::Compiler shaderCompiler(spirvCode32);
		spirv_cross::CompilerGLSL shaderCompiler(pipelineShaderResourceInfos[i].shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode);

		auto resources = shaderCompiler.get_shader_resources();
		auto entry_points = shaderCompiler.get_entry_points_and_stages();
		ParseVertexAttributes(resources, shaderCompiler, pipelineShaderResourceInfos[i].inputAttributesInfo);
		ParseShaderResource(resources, shaderCompiler, pipelineShaderResourceInfos[i].shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT]);
		ParseShaderResource(resources, shaderCompiler, pipelineShaderResourceInfos[i].shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT]);
		ParseShaderResource(resources, shaderCompiler, pipelineShaderResourceInfos[i].shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT]);
		//

	}



}

void ExampleBase::InitShaderStage()
{




}

void ExampleBase::CreateGraphicPipelines()
{
	graphicsPipelines.resize(pipelineShaderResourceInfos.size());
	graphicsPipelineStates.resize(pipelineShaderResourceInfos.size());
	//初始化shader stage
	for (uint32_t pipeID = 0; pipeID < graphicsPipelineStates.size(); pipeID++)
	{
		
	}







}



void ExampleBase::ReadGLSLShaderFile(const std::string& shaderPath, std::vector<char>& shaderCode)
{
	std::string suffix = "";
	suffix = std::filesystem::path(shaderPath).extension().string();
	std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	shaderCode.resize(fileSize);

	file.seekg(0);
	file.read(shaderCode.data(), fileSize);

	file.close();

}

bool ExampleBase::CompileGLSLToSPIRV(VkShaderStageFlagBits shaderStage, const std::vector<char>& srcCode, std::vector<uint32_t>& outSpirvCode)
{
	EShLanguage shaderType = EShLangVertex;
	switch (shaderStage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT: {
		shaderType = EShLanguage::EShLangVertex;
		break;
	}
	case VK_SHADER_STAGE_GEOMETRY_BIT: {
		shaderType = EShLanguage::EShLangGeometry;
		break;
	}
	case VK_SHADER_STAGE_FRAGMENT_BIT: {
		shaderType = EShLanguage::EShLangFragment;
		break;
	}
	case VK_SHADER_STAGE_COMPUTE_BIT: {
		shaderType = EShLanguage::EShLangCompute;
		break;
	}
	default:
		LogFunc(0);
		break;
	}


	// 初始化 glslang
	glslang::InitializeProcess();

	// 编译 GLSL 代码为 SPIR-V 代码
	glslang::TShader shader(shaderType); // 例如，这里假设是一个顶点着色器
	const char* source = srcCode.data();

	shader.setStrings(&source, 1);
	std::cout << "shader compile " << std::endl << std::string(srcCode.begin(), srcCode.end()) << std::endl << std::endl;;
	if (!shader.parse(reinterpret_cast<const TBuiltInResource*>(glslang_default_resource()), 110, false, EShMessages::EShMsgDefault)) {
		std::cerr << "Failed to parse GLSL shader!" << std::endl;
		std::cerr << shader.getInfoLog() << std::endl;
		std::cerr << shader.getInfoDebugLog() << std::endl;
		glslang::FinalizeProcess();
		LogFunc(0);
		return false;
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(EShMsgDefault)) {
		std::cerr << "Failed to link GLSL shader!" << std::endl;
		std::cerr << program.getInfoLog() << std::endl;
		std::cerr << program.getInfoDebugLog() << std::endl;
		glslang::FinalizeProcess();
		LogFunc(0);
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(shaderType), outSpirvCode);

	// 清理 glslang
	glslang::FinalizeProcess();
	return true;
	return false;
}

void ExampleBase::ParseVertexAttributes(const spirv_cross::ShaderResources& srcShaderResource, const spirv_cross::CompilerGLSL& compiler, std::vector<VertexAttributeInfo>& dstCacheAttributeInfo)
{
	//vertex attribute
	dstCacheAttributeInfo.resize(srcShaderResource.stage_inputs.size());
	for (uint32_t attributeIndex = 0; attributeIndex < srcShaderResource.stage_inputs.size(); attributeIndex++)
	{
		auto& vertexAttributeInfo = dstCacheAttributeInfo[attributeIndex];
		auto& shaderVertexAttributeInfo = srcShaderResource.stage_inputs[attributeIndex];
		vertexAttributeInfo.name = shaderVertexAttributeInfo.name;
		vertexAttributeInfo.location = compiler.get_decoration(shaderVertexAttributeInfo.id, spv::DecorationLocation);

		auto type = compiler.get_type(shaderVertexAttributeInfo.base_type_id);
		//inputAttri.columns = type.columns;
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Float: {
			switch (type.vecsize)
			{
			case 1: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32_SFLOAT;
				break;
			}
			case 2: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
				break;
			}
			case 3: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
				break;
			}
			case 4: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			}
			default:
				LogFunc(0);
			}
			break;
		}
		default:
			LogFunc(0);
			break;
		}



	}

}

void ExampleBase::ParseShaderResource(const spirv_cross::ShaderResources& srcShaderResource, const spirv_cross::CompilerGLSL& compiler, ShaderResourceInfo& dstCacheShaderResource)
{
	//解析uniform buffer 信息
	//shaderInfos.shaderUniformBufferInfos.resize(resources.uniform_buffers.size());

	ShaderResourceInfo::DescriptorBinding bindingInfo{ };

	for (uint32_t i = 0; i < srcShaderResource.uniform_buffers.size(); i++)
	{
		auto& uniBuff = srcShaderResource.uniform_buffers[i];
		bindingInfo.name = uniBuff.name;
		//auto& ubInfo = shaderInfos.shaderUniformBufferInfos[i];
		//ubInfo.name = uniBuff.name;
		auto type = compiler.get_type(uniBuff.base_type_id);
		bindingInfo.binding = compiler.get_decoration(uniBuff.id, spv::DecorationBinding);
		bindingInfo.setId = compiler.get_decoration(uniBuff.id, spv::DecorationDescriptorSet);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingInfo.numDescriptor = compiler.get_declared_struct_size(type);
		dstCacheShaderResource.descriptorSetBindings[bindingInfo.setId].push_back(bindingInfo);
	}

	//解析sampler image信息
	for (uint32_t i = 0; i < srcShaderResource.sampled_images.size(); i++)
	{
		auto& samplerImage = srcShaderResource.sampled_images[i];
		auto type = compiler.get_type(samplerImage.base_type_id);
		bindingInfo.setId = compiler.get_decoration(samplerImage.id, spv::Decoration::DecorationDescriptorSet);
		bindingInfo.binding = compiler.get_decoration(samplerImage.id, spv::DecorationBinding);
		bindingInfo.name = samplerImage.name;
		bindingInfo.numDescriptor = compiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

	}



}

