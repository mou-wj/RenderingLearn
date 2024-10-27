#include "ExampleBase.h"
#include "Common/stb_image.h"
#include "Common/stb_image_write.h"



#include <SPIRV/GlslangToSpv.h>
#include <Public/ShaderLang.h>
#include <Include/ResourceLimits.h>
#include <Public/resource_limits_c.h>

#include <shaderc/shaderc.h>
#include <filesystem>
#include <fstream>

using namespace VulkanAPI;


void ExampleBase::Run(ExampleBase* example)
{
	example->InitResourceInfos();
	example->Init();
	example->Loop();



}

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
	InitSubPassInfo();
	InitRenderPass();
	InitFrameBuffer();
	InitGraphicPipelines();
}

void ExampleBase::ParseShaderFiles(const std::vector<ShaderCodePaths>& shaderPaths)
{
	graphcisPipelineInfos.resize(shaderPaths.size());
	std::vector<char> tmpCode;
	for (uint32_t i = 0; i < shaderPaths.size(); i++)
	{
		auto& pipelineShaderResourceInfo = graphcisPipelineInfos[i].pipelineShaderResourceInfo;
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].shaderFilePath = shaderPaths[i].vertexShaderPath;
		//vertex
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths[i].vertexShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode);
		
		ParseSPIRVShaderInputAttribute(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode, pipelineShaderResourceInfo.inputAttributesInfo);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT]);

	
		
		//geom
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT].shaderFilePath = shaderPaths[i].geometryShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths[i].geometryShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT]);
				
		//frag
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT].shaderFilePath = shaderPaths[i].fragmentShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths[i].fragmentShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT]);

	}



}

void ExampleBase::InitContex()
{
	//��������
	window = CreateWin32Window(windowWidth, windowHeight, "MainWindow");

	auto requiredExtension = GetInstanceNeedWinGLFWExtensionNames();
	std::vector<const char*> wantExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
	wantExtensions.insert(wantExtensions.end(), requiredExtension.begin(), requiredExtension.end());
	//����Ƿ���������extension
	auto instanceExtensions = EnumerateExtensionProperties(nullptr);
	uint32_t numSatisfiedExtensions = 0;
	for (uint32_t i = 0; i < instanceExtensions.size(); i++)
	{
		for (uint32_t j = 0; j < wantExtensions.size(); j++)
		{
			if (strcmp(instanceExtensions[i].extensionName, wantExtensions[j]) == 0)
			{
				numSatisfiedExtensions++;
			}
		}


	}
	if (numSatisfiedExtensions != wantExtensions.size())
	{
		Log("ĳ����չ��֧��",0);
	}
	//开启校验层
	auto supportLayers = EnumerateLayerProperties();
	bool supportValidateLayer = false;
	for (uint32_t i = 0; i < supportLayers.size(); i++)
	{
		if (strcmp(supportLayers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0)
		{
			supportValidateLayer = true;
			break;
		}
	}
	
	if (!supportValidateLayer)
	{
		LogFunc(0);
	}

	//auto 

	//����instance
	instance = CreateInstance({ "VK_LAYER_KHRONOS_validation" }, wantExtensions);
	//����surface
	surface = CreateWin32Surface(instance, window);
	debugUtilMessager = CreateDebugInfoMessager(instance);
	PickValidPhysicalDevice();
	physicalDeviceFeatures.geometryShader = VK_TRUE;//����geometry shader


	device = CreateDevice(physicalDevice, { {queueFamilyIndex,{1,1,1}} }, { }, { VK_KHR_SWAPCHAIN_EXTENSION_NAME }, physicalDeviceFeatures);
	graphicQueue = GetQueue(device, queueFamilyIndex, 0);
	presentQueue = GetQueue(device, queueFamilyIndex, 1);
	transferQueue = GetQueue(device, queueFamilyIndex, 2);


	//����swapchain
	swapchain = CreateSwapchain(device, surface, colorFormat, colorSpace, VkExtent2D{ .width = windowWidth,.height = windowHeight }, 1, 1, 2, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex }, swapchainPresentMode);
	
	//����command pool
	commandPool = CreateCommandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndex);
	renderCommandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	toolCommandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);





	auto swapImages = GetSwapchainImages(device, swapchain);
	swapchainImages.resize(swapImages.size());
	for (uint32_t i = 0; i < swapImages.size(); i++)
	{
		swapchainImages[i].image = swapImages[i];
		swapchainImages[i].aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		swapchainImages[i].currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		swapchainImages[i].numLayer = 1;
		swapchainImages[i].numMip = 1;
		swapchainImages[i].extent = VkExtent3D{.width = windowWidth ,.height = windowHeight,.depth = 1};
		swapchainImages[i].sample = VK_SAMPLE_COUNT_1_BIT;
		swapchainImages[i].memory = VK_NULL_HANDLE;
		
		//ת��swapchain image��image layout
		TransferWholeImageLayout(swapchainImages[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);//�ȴ�������Ⱦ���


	}




	













}

void ExampleBase::InitAttanchmentDesc()
{
	auto& colorAttachment = renderTargets.colorAttachment.attachmentDesc;
	colorAttachment.flags = 0;
	colorAttachment.format = colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//����color attachment����Դ
	auto& colorImage = renderTargets.colorAttachment.attachmentImage;
	colorImage = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, colorFormat, windowWidth, windowHeight, 1, 1, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_TILING_OPTIMAL);
	TransferWholeImageLayout(colorImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	renderTargets.colorAttachment.clearValue = VkClearValue{ 0,0,0,1 };



	auto& depthAttachment = renderTargets.depthAttachment.attachmentDesc;
	depthAttachment.flags = 0;
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	auto& depthImage = renderTargets.depthAttachment.attachmentImage;
	depthImage = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, depthFormat, windowWidth, windowHeight, 1, 1, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ,VK_IMAGE_TILING_OPTIMAL);
	TransferWholeImageLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	renderTargets.depthAttachment.clearValue = VkClearValue{ 1.0,0 };





}

void ExampleBase::InitRenderPass()
{
	
	renderPass = CreateRenderPass(device, 0, { renderTargets.colorAttachment.attachmentDesc,renderTargets.depthAttachment.attachmentDesc }, subpassInfo.subpassDescs,subpassInfo.subpassDepends);



}

void ExampleBase::InitFrameBuffer()
{
	frameBuffer = CreateFrameBuffer(device, 0, renderPass, { renderTargets.colorAttachment.attachmentImage.imageView,renderTargets.depthAttachment.attachmentImage.imageView }, windowWidth, windowHeight,1);

}

void ExampleBase::InitDefaultGraphicSubpassInfo()
{
	subpassInfo.subpassDescs.resize(1);
	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.flags = 0;
	subpassDesc1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.inputAttachmentCount = 0;
	subpassDesc1.pInputAttachments = nullptr;
	subpassDesc1.colorAttachmentCount = 1;
	subpassDesc1.pColorAttachments = &renderTargets.colorRef;
	subpassDesc1.pResolveAttachments = nullptr;
	subpassDesc1.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc1.preserveAttachmentCount = 0;
	subpassDesc1.pPreserveAttachments = nullptr;

	subpassInfo.subpassDepends.resize(1);
	auto& subpassDepend1 = subpassInfo.subpassDepends[0];
	subpassDepend1.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDepend1.dstSubpass = 0;
	subpassDepend1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	subpassDepend1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	subpassDepend1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDepend1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDepend1.dependencyFlags =  0;

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
		auto surfaceCapabilities = GetSurfaceCapabilities(physicalDevices[i], surface);
		auto surfaceFormatsCapabilities = GetSurfaceFormats(physicalDevices[i], surface);
		auto queueFamilyProps = GetQueueFamilyProperties(physicalDevices[i]);
		if (familyIndex != -1 && surfaceCapabilities.maxImageCount >=2 && queueFamilyProps[familyIndex].queueCount >=3
			//������Լ��surface�ɵ����Ĵ�С��Χ����Ŀǰ��׼���޸Ĵ�С���Բ����
			)
		{
			//���depth��format
			auto depthFormatFeatures = GetFormatPropetirs(physicalDevices[i], depthFormat);
			if (!(depthFormatFeatures.optimalTilingFeatures & (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)))
			{
				continue;
			}


			//���֧��color��format
			bool surpportColorFormat = false;
			for (auto& surfaceFormatCapabilities : surfaceFormatsCapabilities)
			{
				if (surfaceFormatCapabilities.format == colorFormat )
				{
					surpportColorFormat = true;
					colorSpace = surfaceFormatCapabilities.colorSpace;
					break;
				}

			}
			if (!surpportColorFormat)
			{
				continue;
			}
			bool surpportPresentMode = false;
			auto presentModes = GetSurfacePresentModes(physicalDevices[i], surface);
			for (auto& presentMode : presentModes)
			{
				if (presentMode == swapchainPresentMode)
				{
					surpportPresentMode = true;
					break;
				}

			}
			if (!surpportPresentMode)
			{
				continue;
			}
			auto presentSupport = GetQueueFamilySurfaceSupport(physicalDevices[i], familyIndex, surface);
			if (!presentSupport)
			{
				continue;
			}
			
			//���format�Ƿ�֧��linear tiling
			auto colorFormatProps = GetFormatPropetirs(physicalDevices[i], colorFormat);
			if (!(colorFormatProps.linearTilingFeatures & (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)))
			{
				continue;//���color format��linear tiling ��֧�ֲ�������ɫ�����������������豸
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

int32_t ExampleBase::GetMemoryTypeIndex(uint32_t  wantMemoryTypeBits, VkMemoryPropertyFlags wantMemoryFlags)
{
	auto memoryProperties = GetMemoryProperties(physicalDevice);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (((memoryProperties.memoryTypes[i].propertyFlags & wantMemoryFlags) == wantMemoryFlags) && (wantMemoryTypeBits & ( 1 << i)))
		{
			return i;
		}

	}




	return -1;
}

Image ExampleBase::CreateImage(VkImageType imageType,VkImageViewType viewType,VkFormat format,uint32_t width,uint32_t height, uint32_t depth,uint32_t numMip,uint32_t numLayer,VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags memoryProperies, VkImageTiling tiling,VkSampleCountFlagBits sample, VkImageLayout layout)
{
	Image image;
	image.currentLayout = layout;
	image.sample = sample;
	image.extent = VkExtent3D{ .width = width,.height = height,.depth = depth };
	image.aspect = aspect;
	image.numLayer = numLayer;
	image.numMip = numMip;
	image.tiling = tiling;
	image.image = VulkanAPI::CreateImage(device, 0, imageType, format, image.extent,
		1, 1, image.sample, image.tiling, usage, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex });
	auto imageMemRequiredments = GetImageMemoryRequirments(device, image.image);
	auto memtypeIndex = GetMemoryTypeIndex(imageMemRequiredments.memoryTypeBits, memoryProperies);
	image.memory = AllocateMemory(device, imageMemRequiredments.size, memtypeIndex);
	BindMemoryToImage(device, image.memory, image.image, 0);
	if (memoryProperies & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		image.hostMapPointer = MapMemory(device, image.memory, 0, imageMemRequiredments.size, 0);
	}
	image.imageView = CreateImageView(device, 0, image.image, viewType, format, VkComponentMapping{}, VkImageSubresourceRange{ .aspectMask = aspect,.baseMipLevel = 0,.levelCount = image.numMip ,.baseArrayLayer = 0,.layerCount = image.numLayer });


	return image;
}

Texture ExampleBase::Load2DTexture(const std::string& texFilePath)
{
	int x = 0, y = 0, numChannel = 0;
	auto imageData = stbi_load(texFilePath.c_str(), &x, &y, &numChannel,4);//ǿ�Ƽ���4ͨ�������ݵı���ΪsRGB
	//����texture
	Texture texture;
	texture.image = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, colorFormat, x, y, 1, 1, 1, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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
		imageFaceDatas[i] = (const char*)stbi_load(faceTexFilePaths[i].c_str(), &x, &y, &numChannel, 4);//ǿ�Ƽ���4ͨ�������ݵı���ΪsRGB
	}
	//����texture
	Texture texture;
	texture.image = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_CUBE, colorFormat, x, y, 1, 1, 6, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	texture.sampler = CreateDefaultSampler(device, 1);
	return texture;
}

void ExampleBase::LoadObj(const std::string& objFilePath, Geometry& geo)
{
	//inner data
	geo.geoPath = objFilePath;

	std::string warn, err;
	if (!tinyobj::LoadObj(&geo.vertexAttrib, &geo.shapes, &geo.materials, &warn, &err, objFilePath.c_str()))
	{
		LogFunc(0);
	}

}

Buffer ExampleBase::CreateVertexBuffer(const char* buf, VkDeviceSize size)
{

	return CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

Buffer ExampleBase::CreateIndexBuffer(const char* buf, VkDeviceSize size)
{
	return CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

Buffer ExampleBase::CreateUniformBuffer(const char* buf, VkDeviceSize size)
{
	return CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

void ExampleBase::FillBuffer(Buffer buffer, VkDeviceSize offset, VkDeviceSize size, const char* data)
{
	char* dst = (char*)buffer.hostMapPointer + offset;
	std::memcpy(dst, data, size);

}

void ExampleBase::TransferWholeImageLayout(Image& image, VkImageLayout dstImageLayout)
{
	TransferImageLayout(image.image, image.currentLayout, dstImageLayout, VkImageSubresourceRange{ .aspectMask = image.aspect,.baseMipLevel = 0,.levelCount = image.numMip,.baseArrayLayer = 0,.layerCount = image.numLayer });


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
	
	SubmitCommands(transferQueue, {}, {}, { toolCommandBuffer }, {}, nullptr);
	WaitQueueIdle(transferQueue);
	//WaitSemaphores(device, 0, {transferOperationFinish}, {});



}

void ExampleBase::InitGraphicPipelines()
{
	
	//��ʼ��shader stage
	for (uint32_t pipeID = 0; pipeID < graphcisPipelineInfos.size(); pipeID++)
	{
		graphcisPipelineInfos[pipeID].pipelineStates.Init(windowWidth, windowHeight);
		std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetBindings;
		VkDescriptorSetLayoutBinding binding;
		std::map<VkDescriptorType, uint32_t> needNumDescriptor;
		std::vector<VkVertexInputAttributeDescription> attributeDescs;
		VkVertexInputBindingDescription inputBindingDesc;
		//创建input state
		auto& inputState = graphcisPipelineInfos[pipeID].pipelineStates.vertexInputState;
		attributeDescs.resize(vertexAttributes.size());
		for (uint32_t attriId = 0; attriId < vertexAttributes.size(); attriId++)
		{
			attributeDescs[attriId].binding = 0;//Ĭ��ʹ�ð󶨵�0����Ӧ�󶨵�0��λ�Ķ��㻺��
			attributeDescs[attriId].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescs[attriId].location = attriId;
			attributeDescs[attriId].offset = attriId * 3 * sizeof(float);
		}
		//inputBindingDesc.stride = graphcisPipelineInfos[pipeID].pipelineShaderResourceInfo.vertexIputStride;
		inputBindingDesc.stride = vertexAttributeInputStride;
		inputBindingDesc.binding = 0;//ֻ����һ���󶨵�Ϊ0�İ���Ϣ
		inputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//�Ȳ�ʹ��instance����
		inputState.vertexAttributeDescriptionCount = attributeDescs.size();
		inputState.pVertexAttributeDescriptions = attributeDescs.data();
		inputState.vertexBindingDescriptionCount = 1;
		inputState.pVertexBindingDescriptions = &inputBindingDesc;


		for (const auto& kv : graphcisPipelineInfos[pipeID].pipelineShaderResourceInfo.shaderResourceInfos)
		{
			if (kv.second.spirvCode.size() != 0)
			{



				//����spirv code����shader module
				auto shaderModule = CreateShaderModule(device, 0, kv.second.spirvCode);



				//��ʼ��shader state ��Ϣ
				VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{ };
				pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				pipelineShaderStageCreateInfo.pNext = nullptr;
				pipelineShaderStageCreateInfo.flags = 0;
				pipelineShaderStageCreateInfo.stage = kv.first;
				pipelineShaderStageCreateInfo.module = shaderModule;
				pipelineShaderStageCreateInfo.pName = kv.second.entryName.c_str();
				pipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;
				graphcisPipelineInfos[pipeID].pipelineStates.shaderStages.push_back(pipelineShaderStageCreateInfo);

				//��ȡ�󶨺�set��Ϣ
				//auto& bindins = kv.second.descriptorSetBindings;

				for (auto& setAndBindings : kv.second.descriptorSetBindings)
				{
					auto setId = setAndBindings.first;
					for (auto bindingId = 0;bindingId < setAndBindings.second.size(); bindingId++)
					{
						binding.binding = setAndBindings.second[bindingId].binding;
						binding.descriptorCount = setAndBindings.second[bindingId].numDescriptor;
						binding.descriptorType = setAndBindings.second[bindingId].descriptorType;
						binding.pImmutableSamplers = nullptr;//��ʱ��ʹ�ò����sampler
						binding.stageFlags = kv.first;
						needNumDescriptor[binding.descriptorType] += binding.descriptorCount;
						descriptorSetBindings[setId].push_back(binding);//����descriptor set
					}
					
				}



			}
		}

		//����descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const auto& needDescriptor : needNumDescriptor)
		{
			VkDescriptorPoolSize poolSize;
			poolSize.type = needDescriptor.first;
			poolSize.descriptorCount = needDescriptor.second;
			poolSizes.push_back(poolSize);
		}
		auto descriptorPool = CreateDescriptorPool(device, 0, descriptorSetBindings.size() + 1, poolSizes);
		graphcisPipelineInfos[pipeID].descriptorPool = descriptorPool;


		//����descriptor set
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (const auto& setAndBindingInfos : descriptorSetBindings)
		{
			//����descriptor set layout
			auto descriptorSetLayout = CreateDescriptorSetLayout(device, 0, setAndBindingInfos.second);
			graphcisPipelineInfos[pipeID].descriptorSetInfos[setAndBindingInfos.first].setLayout = descriptorSetLayout;
			auto descriptorSet = AllocateDescriptorSet(device, graphcisPipelineInfos[pipeID].descriptorPool, { descriptorSetLayout });
			graphcisPipelineInfos[pipeID].descriptorSetInfos[setAndBindingInfos.first].descriptorSet = descriptorSet;
			descriptorSetLayouts.push_back(descriptorSetLayout);
		}
		
		//����pipeline layout
		auto pipelineLayout =  CreatePipelineLayout(device, 0, descriptorSetLayouts, {});
		graphcisPipelineInfos[pipeID].pipelineLayout = pipelineLayout;



		//��ʼ��pipeline states
		
		auto& pipelineStates = graphcisPipelineInfos[pipeID].pipelineStates;
		auto pipeline = CreateGraphicsPipeline(device, 0, pipelineStates.shaderStages, &pipelineStates.vertexInputState, &pipelineStates.inputAssemblyState, &pipelineStates.tessellationState, &pipelineStates.viewportState,
			&pipelineStates.rasterizationState, &pipelineStates.multisampleState, &pipelineStates.depthStencilState, &pipelineStates.colorBlendState, &pipelineStates.dynamicState, pipelineLayout,
			renderPass, pipeID, VK_NULL_HANDLE, -1);
		graphcisPipelineInfos[pipeID].pipeline = pipeline;
		

		//delete shader module
		for (const auto& shaderStageInfo : graphcisPipelineInfos[pipeID].pipelineStates.shaderStages)
		{
			DestroyShaderModule(device,shaderStageInfo.module);
		}
	}

}

void ExampleBase::InitSyncObject()
{
	graphicFence = CreateFence(device, 0);
	presentFence = CreateFence(device, 0);
	transferFence = CreateFence(device, 0);

	//����transfer������ɵ�semaphore
	transferOperationFinish = CreateSemaphore(device, 0);

	//����swapchain image valid semaphore

	swapchainImageValidSemaphore = CreateSemaphore(device, 0);

}

void ExampleBase::InitRecources()
{
	InitGeometryResources(geom);
}

void ExampleBase::InitGeometryResources(Geometry& geo)
{
	if (geo.geoPath == "")
	{
		LogFunc(0);
	}
	uint32_t numVertex = geo.vertexAttrib.vertices.size() / 3;
	Geometry geometry;
	geometry.numVertex = numVertex;
	//����vertex buffer
	geometry.vertexBuffer = CreateVertexBuffer(nullptr, vertexAttributeInputStride * numVertex);
	for (uint32_t vertexId = 0; vertexId < numVertex; vertexId++)
	{
		//��䶥��λ������
		FillBuffer(geometry.vertexBuffer, vertexId * vertexAttributeInputStride, 3 * sizeof(float), (const char*)(geo.vertexAttrib.vertices.data() + vertexId * 3));

		//�����������


	}



	geometry.indexBuffers.resize(geo.shapes.size());
	std::vector<uint32_t> indicesData;
	geometry.numIndexPerZone.resize(geometry.indexBuffers.size());
	for (uint32_t i = 0; i < geometry.indexBuffers.size(); i++)
	{

		for (uint32_t cellId; cellId < geo.shapes[i].mesh.num_face_vertices.size(); cellId)
		{
			if (geo.shapes[i].mesh.num_face_vertices[cellId] != 3)
			{
				LogFunc(0);//������������ε�ģ�;�ֱ�ӱ���
			}
		}
		indicesData.resize(geo.shapes[i].mesh.num_face_vertices.size() * 3);
		for (uint32_t attriIndexId = 0; attriIndexId < geo.shapes[i].mesh.indices.size(); attriIndexId++)
		{
			indicesData[attriIndexId] = geo.shapes[i].mesh.indices[attriIndexId].vertex_index;//��Ŷ�������
		}

		geometry.indexBuffers[i] = CreateIndexBuffer((const char*)indicesData.data(), indicesData.size() * sizeof(uint32_t));
		geometry.numIndexPerZone[i] = indicesData.size();

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
	shaderCode.push_back('\0');
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


	// ��ʼ�� glslang
	glslang::InitializeProcess();

	// ���� GLSL ����Ϊ SPIR-V ����
	glslang::TShader shader(shaderType); // ���磬���������һ��������ɫ��
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



	// ���� glslang
	glslang::FinalizeProcess();
		
	return true;
}


void ExampleBase::TransferGLSLFileToSPIRVFileAndRead(const std::string& srcGLSLFile, std::vector<uint32_t>& outSpirvCode)
{
	if (srcGLSLFile == "")
	{
		return;
	}
	std::string vulkanIncludeDir(VULKAN_INCLUDE_DIRS);
	uint32_t pos = vulkanIncludeDir.find_last_of("/");
	std::string vulkanInstallDir = vulkanIncludeDir.substr(0, pos);
	std::string glslcDir = vulkanInstallDir + "/Bin/glslc.exe";
	std::string generateCmd = glslcDir + " " + srcGLSLFile + " -o " + "tmp.spv";
	int ret = system(generateCmd.c_str());
	if (ret != 0)
	{
		LogFunc(0);
	}
	std::ifstream spvfile("tmp.spv", std::ios::ate | std::ios::binary);
	if (!spvfile.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	std::vector<char> spvShaderCode;
	size_t spvfileSize = (size_t)spvfile.tellg();
	spvShaderCode.resize(spvfileSize);

	spvfile.seekg(0);
	spvfile.read(spvShaderCode.data(), spvfileSize);
	spvfile.close();

	std::vector<uint32_t> spirvCode32(
		reinterpret_cast<uint32_t*>(spvShaderCode.data()),
		reinterpret_cast<uint32_t*>(spvShaderCode.data() + spvShaderCode.size()));
	outSpirvCode = spirvCode32;
}

void ExampleBase::ParseSPIRVShaderInputAttribute(const std::vector<uint32_t>& spirvCode, std::vector<ShaderInputAttributeInfo>& dstCacheShaderInputAttributeInfo)
{
	spirv_cross::CompilerGLSL shaderCompiler(spirvCode);

	auto resources = shaderCompiler.get_shader_resources();
	auto entry_points = shaderCompiler.get_entry_points_and_stages();

	//vertex attribute
	dstCacheShaderInputAttributeInfo.resize(resources.stage_inputs.size());
	for (uint32_t attributeIndex = 0; attributeIndex < resources.stage_inputs.size(); attributeIndex++)
	{
		auto& vertexAttributeInfo = dstCacheShaderInputAttributeInfo[attributeIndex];
		auto& shaderVertexAttributeInfo = resources.stage_inputs[attributeIndex];
		vertexAttributeInfo.name = shaderVertexAttributeInfo.name;
		vertexAttributeInfo.location = shaderCompiler.get_decoration(shaderVertexAttributeInfo.id, spv::DecorationLocation);
		auto type = shaderCompiler.get_type(shaderVertexAttributeInfo.base_type_id);
		//inputAttri.columns = type.columns;
		std::string vertexAttributeFormatStr = "";
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Float: {
			switch (type.vecsize)
			{
			case 1: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32_SFLOAT;
				vertexAttributeFormatStr = "VK_FORMAT_R32_SFLOAT";
				break;
			}
			case 2: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
				vertexAttributeFormatStr = "VK_FORMAT_R32G32_SFLOAT";
				break;
			}
			case 3: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
				vertexAttributeFormatStr = "VK_FORMAT_R32G32B32_SFLOAT";
				break;
			}
			case 4: {
				vertexAttributeInfo.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
				vertexAttributeFormatStr = "VK_FORMAT_R32G32B32A32_SFLOAT";
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
		Log("vertex shader input attribute data : " << "location =  " << vertexAttributeInfo.location << ", name :  " <<
			vertexAttributeInfo.name << ", type : " << vertexAttributeFormatStr, vertexAttributeFormatStr == std::string("VK_FORMAT_R32G32B32_SFLOAT"));


	}


}

void ExampleBase::ParseSPIRVShaderResourceInfo(const std::vector<uint32_t>& spirvCode, ShaderResourceInfo& dstCacheShaderResource)
{
	if (spirvCode.size() == 0)
	{
		return;
	}
	spirv_cross::CompilerGLSL shaderCompiler(spirvCode);

	auto resources = shaderCompiler.get_shader_resources();
	auto entry_points = shaderCompiler.get_entry_points_and_stages();
	dstCacheShaderResource.entryName = entry_points[0].name;
	
	//����uniform buffer ��Ϣ
//shaderInfos.shaderUniformBufferInfos.resize(resources.uniform_buffers.size());

	DescriptorBinding bindingInfo{ };

	for (uint32_t i = 0; i < resources.uniform_buffers.size(); i++)
	{
		auto& uniBuff = resources.uniform_buffers[i];
		bindingInfo.name = uniBuff.name;
		//auto& ubInfo = shaderInfos.shaderUniformBufferInfos[i];
		//ubInfo.name = uniBuff.name;
		auto type = shaderCompiler.get_type(uniBuff.base_type_id);
		bindingInfo.binding = shaderCompiler.get_decoration(uniBuff.id, spv::DecorationBinding);
		bindingInfo.setId = shaderCompiler.get_decoration(uniBuff.id, spv::DecorationDescriptorSet);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		dstCacheShaderResource.descriptorSetBindings[bindingInfo.setId].push_back(bindingInfo);
	}

	//����sampler image��Ϣ
	for (uint32_t i = 0; i < resources.sampled_images.size(); i++)
	{
		auto& samplerImage = resources.sampled_images[i];
		auto type = shaderCompiler.get_type(samplerImage.base_type_id);
		bindingInfo.setId = shaderCompiler.get_decoration(samplerImage.id, spv::Decoration::DecorationDescriptorSet);
		bindingInfo.binding = shaderCompiler.get_decoration(samplerImage.id, spv::DecorationBinding);
		bindingInfo.name = samplerImage.name;
		bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

	}


}



Buffer ExampleBase::CreateBuffer(VkBufferUsageFlags usage, const char* buf, VkDeviceSize size, VkMemoryPropertyFlags memoryPropties)
{
	Buffer buffer;
	buffer.usage = usage;
	buffer.size = size;
	buffer.buffer = VulkanAPI::CreateBuffer(device, 0, size, usage, VK_SHARING_MODE_EXCLUSIVE, {queueFamilyIndex});
	//�����ڴ�
	auto memRequirements = GetBufferMemoryRequirements(device, buffer.buffer);
	auto memtypeIndex = GetMemoryTypeIndex(memRequirements.memoryTypeBits, memoryPropties);
	buffer.memory = AllocateMemory(device, size, memtypeIndex);
	buffer.hostMapPointer = MapMemory(device, buffer.memory, 0, size, 0);
	if (buf)
	{
		std::memcpy(buffer.hostMapPointer, buf, size);
	}
	else {
		std::memset(buffer.hostMapPointer, 0, size);
	}

	return buffer;
}

void ExampleBase::CopyImageToImage(Image srcImage, Image dstImage, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& sigSemaphores)
{

	VkImageLayout srcImageOldLayout = srcImage.currentLayout;
	VkImageLayout dstImageOldLayout = dstImage.currentLayout;
	//检查两个image是否尺寸相同
	if (!srcImage.Compatible(dstImage))
	{
		Log("copy : two image are not compatible ! ", 0);
	}

	BeginRecord(toolCommandBuffer, 0);
	//转换image layout
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE;
	imageMemoryBarrier.oldLayout = srcImage.currentLayout;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = srcImage.image;
	imageMemoryBarrier.subresourceRange.aspectMask = srcImage.aspect;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.layerCount = srcImage.numLayer;
	imageMemoryBarrier.subresourceRange.levelCount = srcImage.numMip;
	CmdMemoryBarrier(toolCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, {}, {}, {});
	imageMemoryBarrier.image = dstImage.image;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	CmdMemoryBarrier(toolCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, {}, {}, {});
	
	//拷贝
	VkImageCopy copyRegion{};
	copyRegion.srcOffset = VkOffset3D{ 0,0,0 };
	copyRegion.dstOffset = VkOffset3D{ 0,0,0 };
	std::vector<VkImageCopy> copyRegions;
	for (uint32_t i = 0; i < srcImage.numMip; i++)
	{
		copyRegion.extent = srcImage.GetMipLevelExtent(i);
		copyRegion.srcSubresource = VkImageSubresourceLayers{ .aspectMask = srcImage.aspect,
															  .mipLevel = i,
															  .baseArrayLayer = 0,
															  .layerCount = srcImage.numLayer };
		copyRegion.dstSubresource = copyRegion.srcSubresource;
		copyRegions.push_back(copyRegion);
	}
	CmdCopyImageToImage(toolCommandBuffer, srcImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyRegions);
	

	//转换回去image layout
	imageMemoryBarrier.image = srcImage.image;
	imageMemoryBarrier.newLayout = srcImageOldLayout;
	CmdMemoryBarrier(toolCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, {}, {}, {});
	imageMemoryBarrier.image = dstImage.image;
	imageMemoryBarrier.newLayout = dstImageOldLayout;
	CmdMemoryBarrier(toolCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, {}, {}, {});

	EndRecord(toolCommandBuffer);

	SubmitCommands(transferQueue, waitSemaphores, {}, { toolCommandBuffer }, sigSemaphores, nullptr);

}

uint32_t ExampleBase::GetNextPresentImageIndex()
{
	uint32_t nextImageIndex = VulkanAPI::GetNextValidSwapchainImageIndex(device, swapchain, swapchainImageValidSemaphore, nullptr);


	return nextImageIndex;
}


void ExampleBase::DrawGeom(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& sigSemaphores)
{
	BeginRecord(renderCommandBuffer,0);
	CmdBeginRenderPass(renderCommandBuffer, renderPass, frameBuffer, VkRect2D{ .offset = VkOffset2D{.x = 0 ,.y = 0},.extent = VkExtent2D{.width = windowWidth,.height = windowHeight} }, { renderTargets.colorAttachment.clearValue, renderTargets.depthAttachment.clearValue }, VK_SUBPASS_CONTENTS_INLINE);
	CmdBindVertexBuffers(renderCommandBuffer, 0, { geom.vertexBuffer.buffer }, { 0 });
	for (uint32_t i = 0; i < geom.indexBuffers.size(); i++)
	{
		CmdBindIndexBuffer(renderCommandBuffer, geom.indexBuffers[i].buffer, 0, VK_INDEX_TYPE_UINT32);
		
	}
	//bind pipelines
	for(uint32_t i = 0; i < graphcisPipelineInfos.size();i++)
	{
		CmdBindPipeline(renderCommandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,graphcisPipelineInfos[i].pipeline);
		CmdDrawIndex(renderCommandBuffer, geom.numIndexPerZone[i], 1, 0, 0, 0);
		if (i != graphcisPipelineInfos.size() - 1)
		{
			CmdNextSubpass(renderCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		}
	}
	CmdEndRenderPass(renderCommandBuffer);
	EndRecord(renderCommandBuffer);

	SubmitCommands(graphicQueue, waitSemaphores, {}, { renderCommandBuffer }, sigSemaphores, nullptr);




}

VkResult ExampleBase::Present(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& sigSemaphores, uint32_t swapchainImageIndex)
{
	std::vector<VkResult> res;
	VulkanAPI::Present(presentQueue, waitSemaphores, { swapchain }, { swapchainImageIndex }, res);
	VkResult returnRes;
	returnRes = res[0];
	return returnRes;
}

