#include "ExampleBase.h"
#include "Common/stb_image.h"
#include "Common/stb_image_write.h"
#include "Common/tiny_obj_loader.h"


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
	if (initFlag)
	{
		return;
	}
	initFlag = true;
	Initialize();
	InitContex();
	InitAttanchmentDesc();
	InitFrameBuffer();
	InitRenderPass();
	CreateGraphicPipelines();
}

void ExampleBase::ParseShaderFiles(std::vector<ShaderCodePaths>& shaderPaths)
{
	graphcisPipelineInfos.resize(shaderPaths.size());
	std::vector<char> tmpCode;
	for (uint32_t i = 0; i < shaderPaths.size(); i++)
	{
		auto& pipelineShaderResourceInfo = graphcisPipelineInfos[i].pipelineShaderResourceInfo;
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].shaderFilePath = shaderPaths[i].vertexShaderPath;
		ReadGLSLShaderFile(shaderPaths[i].vertexShaderPath, tmpCode);
		CompileGLSLToSPIRV(VK_SHADER_STAGE_VERTEX_BIT,tmpCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode);
		//解析vertex attribute info
		//spirv_cross::Compiler shaderCompiler(spirvCode32);
		spirv_cross::CompilerGLSL shaderCompiler(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode);

		auto resources = shaderCompiler.get_shader_resources();
		auto entry_points = shaderCompiler.get_entry_points_and_stages();
		ParseVertexAttributes(resources, shaderCompiler, pipelineShaderResourceInfo.inputAttributesInfo);
		ParseShaderResource(resources, shaderCompiler, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT]);
		ParseShaderResource(resources, shaderCompiler, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT]);
		ParseShaderResource(resources, shaderCompiler, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT]);
		//

	}



}

void ExampleBase::InitContex()
{
	//创建窗口
	window = CreateWin32Window(windowWidth, windowHeight, "MainWindow");

	//创建instance
	instance = CreateInstance({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
	//创建surface
	surface = CreateWin32Surface(instance, window);
	
	PickValidPhysicalDevice();
	physicalDeviceFeatures.geometryShader = VK_TRUE;//开启geometry shader
	device = CreateDevice(physicalDevice, { {queueFamilyIndex,3} }, {}, {}, physicalDeviceFeatures);
	graphicQueue = GetQueue(device, queueFamilyIndex, 0);
	presentQueue = GetQueue(device, queueFamilyIndex, 1);
	transferQueue = GetQueue(device, queueFamilyIndex, 2);

	//创建swapchain
	swapchain = CreateSwapchain(device, surface, colorFormat, colorSpace, VkExtent2D{ .width = windowWidth,.height = windowHeight }, 1, 1, 2, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex }, presentMode);
	
	swapchainImages = GetSwapchainImages(device, swapchain);

	//创建swapchain image valid semaphore
	swapchainImageValidSemaphores.resize(swapchainImages.size());
	for (uint32_t i = 0; i < swapchainImageValidSemaphores.size(); i++)
	{
		swapchainImageValidSemaphores[i] = CreateSemaphore(device, 0);
	}
	



	//创建command pool
	commandPool = CreateCommandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndex);
	renderCommandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	toolCommandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);


	//创建transfer操作完成的semaphore
	transferOperationFinish = CreateSemaphore(device, 0);


	//转换swapchain image的image layout
	for (uint32_t i = 0; i < swapchainImages.size(); i++)
	{
		VkImageSubresourceRange subRange;
		subRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subRange.baseArrayLayer = 0;
		subRange.baseMipLevel = 0;
		subRange.layerCount = 1;
		subRange.levelCount = 1;
		TransferImageLayout(swapchainImages[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subRange);//等待拷贝渲染结果
	}








}

int32_t ExampleBase::GetPhysicalDeviceSurportGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice)
{

	auto queueFamilyProperties = GetQueueFamilyProperties(physicalDevice);
	for (int32_t queueFamilyIndex = 0; queueFamilyIndex < static_cast<int32_t>(queueFamilyProperties.size()); queueFamilyIndex++)
	{
		if (queueFamilyProperties[queueFamilyIndex].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT ) )
		{
			return queueFamilyIndex;
		}
	}
	return -1;
}

void ExampleBase::PickValidPhysicalDevice()
{
	auto physicalDevices = EnumeratePhysicalDevice(instance);
	auto physicalDeviceProperties = EnumeratePhysicalDeviceProperties(instance);

	for (uint32_t i = 0; i < physicalDevices.size(); i++)
	{
		auto familyIndex = GetPhysicalDeviceSurportGraphicsQueueFamilyIndex(physicalDevices[i]);
		auto surfaceCapabilities = GetSurfaceCapabilities(physicalDevice, surface);
		auto surfaceFormatsCapabilities = GetSurfaceFormats(physicalDevice, surface);
		
		if (familyIndex != -1 && surfaceCapabilities.maxImageCount >=2 
			//这里可以检查surface可调整的大小范围，但目前不准备修改大小所以不检查
			)
		{
			//检查支持color和depth的format
			bool surpportColorAndDethFormat = false;
			for (auto& surfaceFormatCapabilities : surfaceFormatsCapabilities)
			{
				if (surfaceFormatCapabilities.format & (colorFormat | depthFormat))
				{
					surpportColorAndDethFormat = true;
					colorSpace = surfaceFormatCapabilities.colorSpace;
				}

			}
			if (!surpportColorAndDethFormat)
			{
				continue;
			}
			bool surpportPresentMode = false;
			auto presentModes = GetSurfacePresentModes(physicalDevices[i], surface);
			for (auto& presentMode : presentModes)
			{
				if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
				{
					surpportPresentMode = true;
				}

			}
			if (!surpportPresentMode)
			{
				continue;
			}
			auto presentSupport = GetQueueFamilySurfaceSupport(physicalDevice, familyIndex, surface);
			if (!presentSupport)
			{
				continue;
			}


			queueFamilyIndex = familyIndex;
			physicalDevice = physicalDevices[i];
			physicalDeviceProps = physicalDeviceProperties[i];
			physicalDeviceFeatures= GetPhysicalDeviceFeatures(physicalDevices[i]);
			return;
		}


	}

	LogFunc(0);

}

int32_t ExampleBase::GetMemoryTypeIndex()
{
	auto memoryProperties = GetMemoryProperties(physicalDevice);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		{
			return i;
		}

	}




	return -1;
}

Texture ExampleBase::Load2DTexture(const std::string& texFilePath)
{
	int x = 0, y = 0, numChannel = 0;
	auto imageData = stbi_load(texFilePath.c_str(), &x, &y, &numChannel,4);//强制加载4通道，数据的编码为sRGB
	//创建texture
	Texture texture;
	texture.image = CreateImage(device, 0, VK_IMAGE_TYPE_2D, colorFormat, VkExtent3D{ .width = (uint32_t)x,.height = (uint32_t)y,.depth = 1 },
		1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex });
	texture.imageView = CreateImageView(device, 0, texture.image, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VkComponentMapping{}, VkImageSubresourceRange{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,.baseMipLevel = 0,.levelCount = 1 ,.baseArrayLayer = 0,.layerCount = 1});
	texture.sampler = CreateDefaultSampler(device, 1);
	stbi_image_free(imageData);
	return texture;
}

Texture ExampleBase::LoadCubeTexture(const std::array<std::string, 6>& faceTexFilePaths)
{
	int x = 0, y = 0, numChannel = 0;
	std::array<const char*, 6> imageFaceDatas;
	for (uint32_t i = 0; i < 6; i++)
	{
		imageFaceDatas[i] = (const char*)stbi_load(faceTexFilePaths[i].c_str(), &x, &y, &numChannel, 4);//强制加载4通道，数据的编码为sRGB
	}
	//创建texture
	Texture texture;
	texture.image = CreateImage(device, 0, VK_IMAGE_TYPE_2D, colorFormat, VkExtent3D{ .width = (uint32_t)x,.height = (uint32_t)y,.depth = 1 },
		1, 6, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex });
	texture.imageView = CreateImageView(device, 0, texture.image, VK_IMAGE_VIEW_TYPE_CUBE, colorFormat, VkComponentMapping{}, VkImageSubresourceRange{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,.baseMipLevel = 0,.levelCount = 1 ,.baseArrayLayer = 0,.layerCount = 6 });
	texture.sampler = CreateDefaultSampler(device, 1);
	return texture;
}

Geometry ExampleBase::LoadObj(const std::string& objFilePath)
{
	//inner data
	tinyobj::attrib_t vertexAttrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	if (!tinyobj::LoadObj(&vertexAttrib, &shapes, &materials, &warn, &err, objFilePath.c_str()))
	{
		LogFunc(0);
	}


	return Geometry();
}

Buffer ExampleBase::CreateVertexBuffer(const char* buf, VkDeviceSize size)
{

	Buffer buffer;
	buffer.buffer = CreateBuffer(device, 0, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex });
	//分配内存
	buffer.memory = AllocateMemory(device, size, usedMemoryTypeIndex);
	auto m_ptr = MapMemory(device, buffer.memory, 0, size, 0);
	std::memcpy(m_ptr, buf, size);
	return buffer;
}

Buffer ExampleBase::CreateIndexBuffer(const char* buf, VkDeviceSize size)
{
	Buffer buffer;
	buffer.buffer = CreateBuffer(device, 0, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex });
	//分配内存
	buffer.memory = AllocateMemory(device, size, usedMemoryTypeIndex);
	auto m_ptr = MapMemory(device, buffer.memory, 0, size, 0);
	std::memcpy(m_ptr, buf, size);
	return buffer;
}

void ExampleBase::TransferImageLayout(VkImage image, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkImageSubresourceRange subRange)
{
	BeginRecord(toolCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkImageMemoryBarrier imageMemoryBarrier{ };
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE;
	imageMemoryBarrier.oldLayout = srcImageLayout;
	imageMemoryBarrier.newLayout = dstImageLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subRange;
	CmdMemoryBarrier(toolCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, {}, {}, {});
	EndRecord(toolCommandBuffer);
	
	SubmitCommands(transferQueue, {}, {}, { toolCommandBuffer }, {transferOperationFinish}, nullptr);
	WaitSemaphores(device, 0, {transferOperationFinish}, {});



}

void ExampleBase::CreateGraphicPipelines()
{
	
	//初始化shader stage
	for (uint32_t pipeID = 0; pipeID < graphcisPipelineInfos.size(); pipeID++)
	{
		std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetBindings;
		VkDescriptorSetLayoutBinding binding;
		std::map<VkDescriptorType, uint32_t> needNumDescriptor;
		std::vector<VkVertexInputAttributeDescription> attributeDescs;
		VkVertexInputBindingDescription inputBindingDesc;//只绑定0号位，所以这里只有一个
		for (auto kv : graphcisPipelineInfos[pipeID].pipelineShaderResourceInfo.shaderResourceInfos)
		{
			if (kv.second.spirvCode.size() != 0)
			{
				if (kv.first == VK_SHADER_STAGE_VERTEX_BIT)
				{
					//初始化pipeline的input state
					auto& inputState = graphcisPipelineInfos[pipeID].pipelineStates.vertexInputState;
					auto& shaderAttributeInputInfo = graphcisPipelineInfos[pipeID].pipelineShaderResourceInfo.inputAttributesInfo;
					attributeDescs.resize(shaderAttributeInputInfo.size());
					for (uint32_t attriId = 0; attriId < shaderAttributeInputInfo.size(); attriId++)
					{
						attributeDescs[attriId].binding = 0;//默认使用绑定点0，对应绑定到0号位的定点缓冲
						attributeDescs[attriId].format = shaderAttributeInputInfo[attriId].format;
						attributeDescs[attriId].location = shaderAttributeInputInfo[attriId].location;
						attributeDescs[attriId].offset = shaderAttributeInputInfo[attriId].offset;
					}
					inputBindingDesc.stride = graphcisPipelineInfos[pipeID].pipelineShaderResourceInfo.vertexIputStride;
					inputBindingDesc.binding = 0;//只设置一个绑定点为0的绑定信息
					inputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//先不使用instance绘制

				}


				//根据spirv code创建shader module
				auto shaderModule = CreateShaderModule(device, 0, kv.second.spirvCode);



				//初始化shader state 信息
				VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{ };
				pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				pipelineShaderStageCreateInfo.pNext = nullptr;
				pipelineShaderStageCreateInfo.flags = 0;
				pipelineShaderStageCreateInfo.stage = kv.first;
				pipelineShaderStageCreateInfo.module = shaderModule;
				pipelineShaderStageCreateInfo.pName = kv.second.entryName.c_str();
				pipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;

				//获取绑定和set信息
				//auto& bindins = kv.second.descriptorSetBindings;

				for (auto& setAndBindings : kv.second.descriptorSetBindings)
				{
					auto setId = setAndBindings.first;
					for (auto bindingId = 0;bindingId < setAndBindings.second.size(); bindingId++)
					{
						binding.binding = setAndBindings.second[bindingId].binding;
						binding.descriptorCount = setAndBindings.second[bindingId].numDescriptor;
						binding.descriptorType = setAndBindings.second[bindingId].descriptorType;
						binding.pImmutableSamplers = nullptr;//暂时不使用不变的sampler
						binding.stageFlags = kv.first;
						needNumDescriptor[binding.descriptorType] += binding.descriptorCount;
						descriptorSetBindings[setId].push_back(binding);//插入descriptor set
					}
					
				}



			}
		}

		//创建descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const auto& needDescriptor : needNumDescriptor)
		{
			VkDescriptorPoolSize poolSize;
			poolSize.type = needDescriptor.first;
			poolSize.descriptorCount = needDescriptor.second;
			poolSizes.push_back(poolSize);
		}
		auto descriptorPool = CreateDescriptorPool(device, 0, descriptorSetBindings.size(), poolSizes);
		graphcisPipelineInfos[pipeID].descriptorPool = descriptorPool;


		//分配descriptor set
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (const auto& setAndBindingInfos : descriptorSetBindings)
		{
			//创建descriptor set layout
			auto descriptorSetLayout = CreateDescriptorSetLayout(device, 0, setAndBindingInfos.second);
			graphcisPipelineInfos[pipeID].descriptorSetInfos[setAndBindingInfos.first].setLayout = descriptorSetLayout;
			auto descriptorSet = AllocateDescriptorSet(device, graphcisPipelineInfos[pipeID].descriptorPool, { descriptorSetLayout });
			graphcisPipelineInfos[pipeID].descriptorSetInfos[setAndBindingInfos.first].descriptorSet = descriptorSet;
			descriptorSetLayouts.push_back(descriptorSetLayout);
		}
		
		//创建pipeline layout
		auto pipelineLayout =  CreatePipelineLayout(device, 0, descriptorSetLayouts, {});
		graphcisPipelineInfos[pipeID].pipelineLayout = pipelineLayout;



		//初始化pipeline states
		
		auto& pipelineStates = graphcisPipelineInfos[pipeID].pipelineStates;
		pipelineStates.Init(512, 512);
		auto pipeline = CreateGraphicsPipeline(device, 0, pipelineStates.shaderStages, &pipelineStates.vertexInputState, &pipelineStates.inputAssemblyState, nullptr, &pipelineStates.viewportState,
			&pipelineStates.rasterizationState, nullptr, &pipelineStates.depthStencilState, nullptr, &pipelineStates.dynamicState, pipelineLayout,
			renderPass, pipeID, VK_NULL_HANDLE, 0);
		graphcisPipelineInfos[pipeID].pipeline = pipeline;
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

	DescriptorBinding bindingInfo{ };

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

