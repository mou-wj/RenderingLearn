#include "ExampleBase.h"
#include "Utils/ImageFileTool.h"



#include <SPIRV/GlslangToSpv.h>
#include <Public/ShaderLang.h>
#include <Include/ResourceLimits.h>
#include <Public/resource_limits_c.h>

#include <shaderc/shaderc.h>
#include <filesystem>
#include <fstream>


using namespace VulkanAPI;


ExampleBase::~ExampleBase()
{
	Clear();
}

void ExampleBase::Run(ExampleBase* example)
{
	example->InitResourceInfos();
	example->InitSyncObjectNumInfo();
	example->Init();
	example->Loop();



}

void ExampleBase::BindTexture(const std::string& textureName)
{
	//绑定texture
	ASSERT(textures.find(textureName) != textures.end());
	auto& textureInfo = textureBindInfos[textureName];
	if (!textureInfo.compute)
	{
		auto descriptorType = GetDescriptorType(graphcisPipelineInfos[textureInfo.pipeId].descriptorSetInfos[textureInfo.setId], textureInfo.binding);
		BindTexture(textures[textureName], graphcisPipelineInfos[textureInfo.pipeId].descriptorSetInfos[textureInfo.setId].descriptorSet, textureInfo.binding, textureInfo.elementId, descriptorType);
	}
	else {
		auto descriptorType = GetDescriptorType(computePipelineInfos.descriptorSetInfos[textureInfo.setId], textureInfo.binding);
		BindTexture(textures[textureName], computePipelineInfos.descriptorSetInfos[textureInfo.setId].descriptorSet, textureInfo.binding, textureInfo.elementId, descriptorType);
	}
	
}

void ExampleBase::BindBuffer(const std::string& bufferName)
{
	ASSERT(buffers.find(bufferName) != buffers.end());
	auto& bufferInfo = bufferBindInfos[bufferName];
	if (!bufferInfo.compute)
	{
		auto descriptorType = GetDescriptorType(graphcisPipelineInfos[bufferInfo.pipeId].descriptorSetInfos[bufferInfo.setId], bufferInfo.binding);
		BindBuffer(buffers[bufferName], graphcisPipelineInfos[bufferInfo.pipeId].descriptorSetInfos[bufferInfo.setId].descriptorSet, bufferInfo.binding, bufferInfo.elementId, descriptorType);
	}
	else {
		auto descriptorType = GetDescriptorType(computePipelineInfos.descriptorSetInfos[bufferInfo.setId], bufferInfo.binding);
		BindBuffer(buffers[bufferName], computePipelineInfos.descriptorSetInfos[bufferInfo.setId].descriptorSet, bufferInfo.binding, bufferInfo.elementId, descriptorType);
	}
	
}

void ExampleBase::Init()
{
	if (initFlag)
	{
		return;
	}
	InitSubPassInfo();
	ParseShaderFiles();
	initFlag = true;
	Initialize();
	InitContex();
	InitAttanchmentDesc();
	InitRenderPass();
	InitFrameBuffer();
	InitGraphicPipelines();
	InitQueryPool();
	InitSyncObject();
	InitRecources();
	InitComputeInfo();
	InitCompute();
}

void ExampleBase::ParseShaderFiles()
{

	graphcisPipelineInfos.resize(subpassInfo.subpassDescs.size());
	std::vector<char> tmpCode;
	for (uint32_t i = 0; i < subpassInfo.subpassDescs.size(); i++)
	{
		const auto& shaderPaths = subpassInfo.subpassDescs[i].pipelinesShaderCodePaths;
		auto& pipelineShaderResourceInfo = graphcisPipelineInfos[i].pipelineShaderResourceInfo;
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].shaderFilePath = shaderPaths.vertexShaderPath;
		//vertex
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.vertexShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode);
		
		ParseSPIRVShaderInputAttribute(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode, pipelineShaderResourceInfo.inputAttributesInfo);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_VERTEX_BIT]);

	
		
		//geom
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT].shaderFilePath = shaderPaths.geometryShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.geometryShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_GEOMETRY_BIT]);
				
		//frag
		pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT].shaderFilePath = shaderPaths.fragmentShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.fragmentShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[VK_SHADER_STAGE_FRAGMENT_BIT]);

	}



}

void ExampleBase::InitContex()
{
	//��������
	window = CreateWin32Window(windowWidth, windowHeight, "MainWindow");

	//绑定回调
	WindowEventHandler::BindWindow(window);

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

	physicalDeviceFeatures.depthClamp ;
	device = CreateDevice(physicalDevice, { {queueFamilyIndex,{1}} }, { }, { VK_KHR_SWAPCHAIN_EXTENSION_NAME }, physicalDeviceFeatures);
	graphicQueue = GetQueue(device, queueFamilyIndex, 0);

	//����command pool
	commandPool = CreateCommandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndex);
	InitCommandList();

	//����swapchain
	swapchain = CreateSwapchain(device, surface, colorFormat, colorSpace, VkExtent2D{ .width = windowWidth,.height = windowHeight }, 1, 1, 2, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex }, swapchainPresentMode);


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
		swapchainImages[i].format = colorFormat;
		//ת��swapchain image��image layout
		TransferWholeImageLayout(swapchainImages[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);//�ȴ�������Ⱦ���


	}


	













}

void ExampleBase::InitAttanchmentDesc()
{
	//����color attachment����Դ
	auto& colorImage = renderTargets.colorAttachment.attachmentImage;
	colorImage = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, colorFormat, windowWidth, windowHeight, 1, 1, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkComponentMapping{}, VK_IMAGE_TILING_OPTIMAL);
	//TransferWholeImageLayout(colorImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	renderTargets.colorAttachment.clearValue = VkClearValue{ 0,0,0,1 };


	auto& colorAttachment = renderTargets.colorAttachment.attachmentDesc;
	colorAttachment.flags = 0;
	colorAttachment.format = colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = renderTargets.colorAttachment.attachmentImage.currentLayout;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	renderTargets.colorAttachment.attachmentImage.currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	auto& depthImage = renderTargets.depthAttachment.attachmentImage;
	depthImage = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, depthFormat, windowWidth, windowHeight, 1, 1, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkComponentMapping{}, VK_IMAGE_TILING_OPTIMAL);
	//TransferWholeImageLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	renderTargets.depthAttachment.clearValue = VkClearValue{ 1.0,0 };



	auto& depthAttachment = renderTargets.depthAttachment.attachmentDesc;
	depthAttachment.flags = 0;
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = renderTargets.depthAttachment.attachmentImage.currentLayout;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	renderTargets.depthAttachment.attachmentImage.currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;





}

void ExampleBase::InitRenderPass()
{
	std::vector<VkSubpassDescription> subpassDescriptions;
	for (uint32_t i = 0; i < subpassInfo.subpassDescs.size(); i++)
	{
		subpassDescriptions.push_back(subpassInfo.subpassDescs[i].subpassDescription);
	}
	renderPass = CreateRenderPass(device, 0, { renderTargets.colorAttachment.attachmentDesc,renderTargets.depthAttachment.attachmentDesc }, subpassDescriptions,subpassInfo.subpassDepends);



}

void ExampleBase::InitFrameBuffer()
{
	frameBuffer = CreateFrameBuffer(device, 0, renderPass, { renderTargets.colorAttachment.attachmentImage.imageView,renderTargets.depthAttachment.attachmentImage.imageView }, windowWidth, windowHeight,1);

}

void ExampleBase::InitSyncObject()
{
	//CreateFence(device,numFences);
	CreateSemaphores(numSemaphores);
}

void ExampleBase::InitDefaultGraphicSubpassInfo(ShaderCodePaths subpassShaderCodePaths)
{
	subpassInfo.subpassDescs.resize(1);
	subpassInfo.subpassDescs[0].subpassPipelineStates.Init(windowWidth, windowHeight);
	subpassInfo.subpassDescs[0].pipelinesShaderCodePaths = subpassShaderCodePaths;
	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.subpassDescription.flags = 0;
	subpassDesc1.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.subpassDescription.inputAttachmentCount = 0;
	subpassDesc1.subpassDescription.pInputAttachments = nullptr;
	subpassDesc1.subpassDescription.colorAttachmentCount = 1;
	subpassDesc1.subpassDescription.pColorAttachments = &renderTargets.colorRef;
	subpassDesc1.subpassDescription.pResolveAttachments = nullptr;
	subpassDesc1.subpassDescription.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc1.subpassDescription.preserveAttachmentCount = 0;
	subpassDesc1.subpassDescription.pPreserveAttachments = nullptr;

	subpassInfo.subpassDepends.resize(1);
	auto& subpassDepend1 = subpassInfo.subpassDepends[0];
	subpassDepend1.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDepend1.dstSubpass = 0;
	//subpassDepend1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subpassDepend1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ;
	//subpassDepend1.srcAccessMask = 0;
	//subpassDepend1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDepend1.dependencyFlags =  0;

	subpassDepend1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDepend1.srcAccessMask = 0;
	subpassDepend1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDepend1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

}


int32_t ExampleBase::GetSuitableQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags wantQueueFlags, bool needSupportPresent, uint32_t wantNumQueue)
{
	auto queueFamilyProperties = GetQueueFamilyProperties(physicalDevice);
	
	for (int32_t queueFamilyIndex = 0; queueFamilyIndex < static_cast<int32_t>(queueFamilyProperties.size()); queueFamilyIndex++)
	{
		bool prenset = GetQueueFamilySurfaceSupport(physicalDevice, queueFamilyIndex, surface);
		if (((queueFamilyProperties[queueFamilyIndex].queueFlags & wantQueueFlags) == wantQueueFlags) && (queueFamilyProperties[queueFamilyIndex].queueCount >= wantNumQueue) && (needSupportPresent ? prenset : true) )
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
		auto familyIndex = GetSuitableQueueFamilyIndex(physicalDevices[i], VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |VK_QUEUE_TRANSFER_BIT, true, 1);;
		auto surfaceCapabilities = GetSurfaceCapabilities(physicalDevices[i], surface);
		
		auto queueFamilyProps = GetQueueFamilyProperties(physicalDevices[i]);
		if (familyIndex != -1 && surfaceCapabilities.maxImageCount >=2 
			//������Լ��surface�ɵ����Ĵ�С��Χ����Ŀǰ��׼���޸Ĵ�С���Բ����
			)
		{
			//���depth��format
			auto depthFormatFeatures = GetFormatPropetirs(physicalDevices[i], depthFormat);
			if (!(depthFormatFeatures.optimalTilingFeatures & (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)))
			{
				continue;
			}

			//���format�Ƿ�֧��linear tiling
			auto colorFormatProps = GetFormatPropetirs(physicalDevices[i], colorFormat);
			uint64_t wantFormatFeature = (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT );
			if (!((colorFormatProps.linearTilingFeatures & wantFormatFeature) == wantFormatFeature))
			{
				continue;//���color format��linear tiling ��֧�ֲ�������ɫ�����������������豸
			}
			bool suitableTextxureFormatFound = false;
			uint64_t textureWantFormatFeature = (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
			for (const auto& candidatedFormat : candidatedTextureFormats)
			{
				auto textureFormatProps = GetFormatPropetirs(physicalDevices[i], candidatedFormat);
				
				if ((textureFormatProps.linearTilingFeatures & textureWantFormatFeature) == textureWantFormatFeature)
				{
					suitableTextxureFormatFound = true;
					textureFormat = candidatedFormat;
					break;
				
				}
			}
			if (!suitableTextxureFormatFound)
			{
				continue;
			}



			auto surfaceFormatsCapabilities = GetSurfaceFormats(physicalDevices[i], surface);
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

Image ExampleBase::CreateImage(VkImageType imageType,VkImageViewType viewType,VkFormat format,uint32_t width,uint32_t height, uint32_t depth,uint32_t numMip,uint32_t numLayer,VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags memoryProperies, VkComponentMapping viewMapping, VkImageTiling tiling,VkSampleCountFlagBits sample, VkImageLayout layout)
{
	Image image;
	image.currentLayout = layout;
	image.sample = sample;
	image.extent = VkExtent3D{ .width = width,.height = height,.depth = depth };
	image.aspect = aspect;
	image.numLayer = numLayer;
	image.numMip = numMip;
	image.tiling = tiling;
	image.format = format;
	VkImageCreateFlags imageCreatFlags = 0;
	if (viewType == VK_IMAGE_VIEW_TYPE_CUBE || viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
	{
		imageCreatFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	//检查格式支持
	//auto formatProperties = GetImageFormatProperties(physicalDevice, image.format, imageType, image.tiling, usage, imageCreatFlags);
	//if (formatProperties.maxArrayLayers == 0)
	//{
	//	ASSERT(0);
	//}

	image.image = VulkanAPI::CreateImage(device, imageCreatFlags, imageType, image.format, image.extent,
		image.numMip, image.numLayer, image.sample, image.tiling, usage, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex });
	auto imageMemRequiredments = GetImageMemoryRequirments(device, image.image);
	auto memtypeIndex = GetMemoryTypeIndex(imageMemRequiredments.memoryTypeBits, memoryProperies);
	image.memory = AllocateMemory(device, imageMemRequiredments.size, memtypeIndex);
	BindMemoryToImage(device, image.memory, image.image, 0);
	if (memoryProperies & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		image.hostMapPointer = MapMemory(device, image.memory, 0, imageMemRequiredments.size, 0);
	}
	image.imageView = CreateImageView(device, 0, image.image,usage, viewType, format, viewMapping, VkImageSubresourceRange{ .aspectMask = aspect,.baseMipLevel = 0,.levelCount = image.numMip ,.baseArrayLayer = 0,.layerCount = image.numLayer });

	return image;
}

void ExampleBase::FillImage(Image& image, uint32_t layer, uint32_t mip, VkImageAspectFlags aspect, uint32_t width, uint32_t height, uint32_t numComponets, const char* data)
{
	VkImageSubresource subresource;
	subresource.aspectMask = aspect;
	subresource.mipLevel = mip;
	subresource.arrayLayer = layer;
	auto sublayout = GetImageSubresourceLayout(device, image.image, subresource);
	VkDeviceSize rowOffset = 0;
	for (uint32_t i = 0; i < height; i++)
	{
		//填充每一行
		rowOffset = sublayout.offset +  i * sublayout.rowPitch;
		FillImage(image, rowOffset, width * numComponets, data + i * width * numComponets);


	}

	//WriteJpeg("textttt.jpg", (const char*)image.hostMapPointer, sublayout.rowPitch / 4, sublayout.size / sublayout.rowPitch);
}

void ExampleBase::FillImage(Image& image, VkDeviceSize offset, VkDeviceSize size, const char* data)
{
	char* dst = (char*)image.hostMapPointer + offset;
	std::memcpy(dst, data, size);
}

void ExampleBase::DestroyImage(const Image& image)
{
	VulkanAPI::DestroyImage(device, image.image);
	DestroyImageView(device, image.imageView);
	ReleaseMemory(device, image.memory);
}



Texture ExampleBase::CreateTexture(TextureBindInfo& textureBindInfo)
{
	Texture texture;
	ASSERT(textureBindInfo.textureDataSources.size());
	uint32_t x = textureBindInfo.textureDataSources[0].width, y = textureBindInfo.textureDataSources[0].height;
	bool flip = false;
	if (textureBindInfo.viewType == VK_IMAGE_VIEW_TYPE_CUBE) {
		flip = true;
	}
	//加载数据
	for (uint32_t i = 0; i < textureBindInfo.textureDataSources.size(); i++)
	{
		if (textureBindInfo.textureDataSources[i].picturePath != "")
		{
			LoadCharUnsignedCharJpeg(textureBindInfo.textureDataSources[i].picturePath, { R,G,B,A }, textureBindInfo.textureDataSources[i].imagePixelDatas, x, y, flip);
			textureBindInfo.textureDataSources[i].width = x;
			textureBindInfo.textureDataSources[i].height = y;
			if (i !=0)
			{
				ASSERT((x == textureBindInfo.textureDataSources[i - 1].width) &&
					(y == textureBindInfo.textureDataSources[i - 1].height));
			}

		}

	}
	//����texture
	uint32_t numLayer = textureBindInfo.textureDataSources.size();
	texture.image = CreateImage(VK_IMAGE_TYPE_2D, textureBindInfo.viewType, textureFormat, x, y, 1, 1, numLayer, textureBindInfo.usage, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VkComponentMapping{},VK_IMAGE_TILING_LINEAR);
	texture.sampler = CreateDefaultSampler(device, 1);
	for (uint32_t i = 0; i < numLayer; i++)
	{
		x = textureBindInfo.textureDataSources[i].width;
		y = textureBindInfo.textureDataSources[i].height;
		FillImage(texture.image, i, 0, VK_IMAGE_ASPECT_COLOR_BIT, x, y, 4, textureBindInfo.textureDataSources[i].imagePixelDatas.data());
		
	}

	TransferWholeImageLayout(texture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

Buffer ExampleBase::CreateShaderAccessBuffer(const char* buf, VkDeviceSize size)
{
	return CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

void ExampleBase::FillBuffer(Buffer buffer, VkDeviceSize offset, VkDeviceSize size, const char* data)
{
	char* dst = (char*)buffer.hostMapPointer + offset;
	std::memcpy(dst, data, size);

}

void ExampleBase::CmdListReset(CommandList& cmdList)
{
	CommandBufferReset(cmdList.commandBuffer);
}

void ExampleBase::CmdListRecordBegin(CommandList& cmdList)
{
	BeginRecord(cmdList.commandBuffer, cmdList.commandBufferUsage);
}

void ExampleBase::CmdListRecordEnd(CommandList& cmdList)
{
	EndRecord(cmdList.commandBuffer);
}



void ExampleBase::CmdOpsImageMemoryBarrer(CommandList& cmdList, Image& image, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout dstImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	auto imageBarrier = barrier.ImageBarrier(image, srcAccess, dstAccess, dstImageLayout);
	CmdMemoryBarrier(cmdList.commandBuffer, srcStageMask, dstStageMask, 0, {}, {}, { imageBarrier });
	image.currentLayout = dstImageLayout;
}

void ExampleBase::CmdOpsDispatch(CommandList& cmdList, std::array<uint32_t, 3> groupSize)
{
	CmdBindPipeline(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineInfos.pipeline);
	//绑定描述符集
	for (const auto& setInfo : computePipelineInfos.descriptorSetInfos)
	{
		CmdBindDescriptorSet(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineInfos.pipelineLayout, setInfo.first, { setInfo.second.descriptorSet }, {});
	}
	CmdDispatch(cmdList.commandBuffer, groupSize[0], groupSize[1], groupSize[2]);


}


void ExampleBase::CmdOpsCopyWholeImageToImage(CommandList& cmdList,Image srcImage, Image dstImage)
{
	//检查两个image是否尺寸相同
	if (!srcImage.WholeCopyCompatible(dstImage))
	{
		Log("copy : two image are not compatible ! ", 0);
	}
	VkImageLayout srcOldLayout = srcImage.currentLayout, dstOldLayout = dstImage.currentLayout;
	//转换image layout
	//CmdOpsImageMemoryBarrer(cmdList, srcImage, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	//CmdOpsImageMemoryBarrer(cmdList, dstImage, VK_ACCESS_MEMORY_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

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
	if (srcImage.currentLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL || dstImage.currentLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		ASSERT(0);
	}
	CmdCopyImageToImage(cmdList.commandBuffer, srcImage.image, srcImage.currentLayout, dstImage.image, dstImage.currentLayout, copyRegions);


	////转换回去image layout
	//CmdOpsImageMemoryBarrer(cmdList, srcImage, srcOldAccess, srcOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	//CmdOpsImageMemoryBarrer(cmdList, dstImage, dstOldAccess, dstOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);


}

void ExampleBase::CmdOpsBlitWholeImageToImage(CommandList& cmdList, Image srcImage, Image dstImage)
{
	VkImageBlit blitRegion{};
	blitRegion.srcOffsets[0] = VkOffset3D{ 0,0,0 };

	blitRegion.dstOffsets[0] = VkOffset3D{ 0,0,0 };
	//拷贝
	std::vector<VkImageBlit> blitRegions;
	for (uint32_t i = 0; i < srcImage.numMip; i++)
	{
		blitRegion.srcSubresource = VkImageSubresourceLayers{ .aspectMask = srcImage.aspect,
															  .mipLevel = i,
															  .baseArrayLayer = 0,
															  .layerCount = srcImage.numLayer };
		auto srcMipSize = srcImage.GetMipLevelExtent(i);
		blitRegion.srcOffsets[1] = VkOffset3D{ (int32_t)srcMipSize.width,(int32_t)srcMipSize.width,(int32_t)srcMipSize.depth };
		auto dstMipSize = dstImage.GetMipLevelExtent(i);
		blitRegion.dstOffsets[1] = VkOffset3D{ (int32_t)dstMipSize.width,(int32_t)dstMipSize.width,(int32_t)dstMipSize.depth };
		blitRegion.dstSubresource = blitRegion.srcSubresource;
		blitRegions.push_back(blitRegion);
	}
	if (srcImage.currentLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL || dstImage.currentLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		ASSERT(0);
	}
	CmdBlitImageToImage(cmdList.commandBuffer, srcImage.image, srcImage.currentLayout, dstImage.image, dstImage.currentLayout, blitRegions);

}

void ExampleBase::CmdOpsDrawGeom(CommandList& cmdList)
{
	//
	//if (renderTargets.colorAttachment.attachmentImage.currentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || renderTargets.depthAttachment.attachmentImage.currentLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	//{
	//	ASSERT(0);
	//}
	

	ASSERT(subpassDrawGeoInfos.size() != 0)
	CmdBeginRenderPass(cmdList.commandBuffer, renderPass, frameBuffer, VkRect2D{ .offset = VkOffset2D{.x = 0 ,.y = 0},.extent = VkExtent2D{.width = windowWidth,.height = windowHeight} }, { renderTargets.colorAttachment.clearValue, renderTargets.depthAttachment.clearValue }, VK_SUBPASS_CONTENTS_INLINE);
	for (uint32_t curSubpassIndex = 0; curSubpassIndex < subpassDrawGeoInfos.size(); curSubpassIndex++)
	{			
		//bind pipelines
		CmdBindPipeline(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphcisPipelineInfos[curSubpassIndex].pipeline);
		//绑定描述符集
		for (const auto& setInfo : graphcisPipelineInfos[curSubpassIndex].descriptorSetInfos)
		{
			CmdBindDescriptorSet(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphcisPipelineInfos[curSubpassIndex].pipelineLayout, setInfo.first, { setInfo.second.descriptorSet }, {});
		}
		
		for (uint32_t i = 0; i < subpassDrawGeoInfos[curSubpassIndex].size(); i++)
		{
			const auto& geom = geoms[subpassDrawGeoInfos[curSubpassIndex][i]];
			if (geom.useIndexBuffers)
			{

				CmdBindVertexBuffers(cmdList.commandBuffer, 0, { geom.vertexBuffer.buffer }, { 0 });
				for (uint32_t i = 0; i < geom.indexBuffers.size(); i++)
				{
					CmdBindIndexBuffer(cmdList.commandBuffer, geom.indexBuffers[i].buffer, 0, VK_INDEX_TYPE_UINT32);
					CmdDrawIndex(cmdList.commandBuffer, geom.numIndexPerZone[i], 1, 0, 0, 0);
				}


			}
			else {
				for (uint32_t i = 0; i < geom.shapeVertexBuffers.size(); i++)
				{
					CmdBindVertexBuffers(cmdList.commandBuffer, 0, { geom.shapeVertexBuffers[i].buffer }, { 0 });
					CmdDrawVertex(cmdList.commandBuffer, geom.numIndexPerZone[i], 1, 0, 0);
				}



			
			
			}



		}

		if (curSubpassIndex != subpassDrawGeoInfos.size() - 1)
		{
			CmdNextSubpass(cmdList.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	CmdEndRenderPass(cmdList.commandBuffer);
	


}

void ExampleBase::CmdListSubmit(CommandList& cmdList, SubmitSynchronizationInfo& info)
{
	SubmitCommands(graphicQueue, info.waitSemaphores, info.waitStages, { cmdList.commandBuffer }, info.sigSemaphores, cmdList.commandFinishFence);
}

void ExampleBase::CmdListWaitFinish(CommandList& cmdList)
{
	WaitAllFence({ cmdList.commandFinishFence });
	ResetAllFence({ cmdList.commandFinishFence });
}

VkResult ExampleBase::Present(uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores)
{
	std::vector<VkResult> res;
	VulkanAPI::Present(graphicQueue, waitSemaphores, { swapchain }, { imageIndex }, res);
	VkResult returnRes;
	returnRes = res[0];
	if (returnRes != VK_SUCCESS)
	{
		ASSERT(0);
	}
	return returnRes;
}





void ExampleBase::DestroyBuffer(Buffer& buffer)
{
	VulkanAPI::DestroyBuffer(device, buffer.buffer);
	ReleaseMemory(device, buffer.memory);
}


void ExampleBase::TransferWholeImageLayout(Image& image, VkImageLayout dstImageLayout)
{
	CmdListWaitFinish(oneSubmitCommandList);
	CmdListRecordBegin(oneSubmitCommandList);
	CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_NONE, VK_ACCESS_NONE, dstImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	CmdListRecordEnd(oneSubmitCommandList);
	
	SubmitSynchronizationInfo info;
	CmdListSubmit(oneSubmitCommandList, info);


}

void ExampleBase::InitGraphicPipelines()
{
	
	//��ʼ��shader stage
	for (uint32_t pipeID = 0; pipeID < graphcisPipelineInfos.size(); pipeID++)
	{
		graphcisPipelineInfos[pipeID].pipelineStates = subpassInfo.subpassDescs[pipeID].subpassPipelineStates;
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
			graphcisPipelineInfos[pipeID].descriptorSetInfos[setAndBindingInfos.first].bindings = descriptorSetBindings[setAndBindingInfos.first];
		}
		
		//����pipeline layout
		auto pipelineLayout =  CreatePipelineLayout(device, 0, descriptorSetLayouts, {});
		graphcisPipelineInfos[pipeID].pipelineLayout = pipelineLayout;



		//��ʼ��pipeline states
		
		auto& pipelineStates = graphcisPipelineInfos[pipeID].pipelineStates;
		auto pipeline = CreateGraphicsPipeline(device, 0, pipelineStates.shaderStages, &pipelineStates.vertexInputState, &pipelineStates.inputAssemblyState, &pipelineStates.tessellationState, &pipelineStates.viewportState,
			&pipelineStates.rasterizationState, &pipelineStates.multisampleState, &pipelineStates.depthStencilState, &pipelineStates.colorBlendState, &pipelineStates.dynamicState, pipelineLayout,
			renderPass, pipeID);
		graphcisPipelineInfos[pipeID].pipeline = pipeline;
		

		//delete shader module
		for (const auto& shaderStageInfo : graphcisPipelineInfos[pipeID].pipelineStates.shaderStages)
		{
			DestroyShaderModule(device,shaderStageInfo.module);
		}
	}

}


void ExampleBase::InitRecources()
{
	for (uint32_t i = 0; i < geoms.size(); i++)
	{
		InitGeometryResources(geoms[i]);
	}

	InitTextureResources();

	InitUniformBufferResources();


}

void ExampleBase::InitQueryPool()
{
	queryPool = CreateQueryPool(device, 0, VK_QUERY_TYPE_PIPELINE_STATISTICS, 1, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT);


}

void ExampleBase::InitCommandList()
{
	graphicCommandList.commandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	graphicCommandList.commandBufferUsage = 0;
	graphicCommandList.commandFinishFence = CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT);
	oneSubmitCommandList.commandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	oneSubmitCommandList.commandBufferUsage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	oneSubmitCommandList.commandFinishFence = CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT);

}

void ExampleBase::Clear()
{
	//清除所有vulkan资源
	DeviceWaitIdle(device);
	ClearGraphicPipelines();
	ClearComputePipeline();
	ClearRenderPass();
	ClearFrameBuffer();
	ClearAttanchment();
	ClearRecources();
	ClearSyncObject();
	ClearQueryPool();

	ClearContex();

}

void ExampleBase::ClearContex()
{				
	DesctroyCommandPool(device, commandPool);

	DestroySwapchain(device, swapchain);
	DestroyDevice(device);
	DestroySurface(instance, surface);
	DestroyGLFWWin32Window(window);
	DestroyDebugInfoMessager(instance,debugUtilMessager);
	DestroyInstance(instance);
}				  
				  
void ExampleBase::ClearAttanchment()
{				  
	DestroyImage(renderTargets.colorAttachment.attachmentImage);
	DestroyImage(renderTargets.depthAttachment.attachmentImage);
}				  
				  
void ExampleBase::ClearRenderPass()
{			
	DestroyRenderPass(device, renderPass);

}				  
				  
void ExampleBase::ClearFrameBuffer()
{				 
	DestroyFrameBuffer(device, frameBuffer);
}				  
				  
void ExampleBase::ClearGraphicPipelines()
{				 
	for (uint32_t i = 0; i < graphcisPipelineInfos.size(); i++)
	{

		DestroyPipeline(device, graphcisPipelineInfos[i].pipeline);
		DestroyPipelineLayout(device, graphcisPipelineInfos[i].pipelineLayout);
		for (uint32_t setId = 0; setId < graphcisPipelineInfos[i].descriptorSetInfos.size(); setId++)
		{
			//ReleaseDescriptorSets(device, graphcisPipelineInfos[i].descriptorPool, { graphcisPipelineInfos[i].descriptorSetInfos[setId].descriptorSet });
			DestroyDesctriptorSetLayout(device, graphcisPipelineInfos[i].descriptorSetInfos[setId].setLayout);
		}

		DestroyDescriptorPool(device,graphcisPipelineInfos[i].descriptorPool);
	}


}				  
				  
void ExampleBase::ClearSyncObject()
{				  
	for (uint32_t i = 0; i < semaphores.size(); i++)
	{
		DestroySemaphore(device, semaphores[i]);
	}
	//for (uint32_t i = 0; i < fences.size(); i++)
	//{
	//	DestroyFence(device, fences[i]);
	//}
	DestroyFence(device, oneSubmitCommandList.commandFinishFence);
	DestroyFence(device, graphicCommandList.commandFinishFence);
}				  
				  
void ExampleBase::ClearRecources()
{			
	//清除textures
	for (auto iter = textures.begin(); iter != textures.end(); iter++)
	{
		DestroyImage((*iter).second.image);
		DestroySampler(device, (*iter).second.sampler);
	}

	//清除geometry
	for (uint32_t i = 0; i < geoms.size(); i++)
	{
		DestroyBuffer(geoms[i].vertexBuffer);
		for (uint32_t zoneId = 0; zoneId < geoms[i].indexBuffers.size(); zoneId++)
		{
			DestroyBuffer(geoms[i].indexBuffers[zoneId]);
		}
	}

	//清除Uniform buffer
	for (auto& buffer : buffers)
	{
		DestroyBuffer(buffer.second);
	}


}				  
				  
void ExampleBase::ClearQueryPool()
{
	DestroyQueryPool(device, queryPool);
}

void ExampleBase::ClearComputePipeline()
{
	if (computeDesc.valid)
	{
		DestroyPipeline(device, computePipelineInfos.pipeline);
		DestroyPipelineLayout(device, computePipelineInfos.pipelineLayout);
		for (uint32_t setId = 0; setId < computePipelineInfos.descriptorSetInfos.size(); setId++)
		{
			//ReleaseDescriptorSets(device, graphcisPipelineInfos[i].descriptorPool, { graphcisPipelineInfos[i].descriptorSetInfos[setId].descriptorSet });
			DestroyDesctriptorSetLayout(device, computePipelineInfos.descriptorSetInfos[setId].setLayout);
		}
		DestroyDescriptorPool(device, computePipelineInfos.descriptorPool);
	}

}






void ExampleBase::InitGeometryResources(Geometry& geo)
{
	if (geo.geoPath == "")
	{
		LogFunc(0);
	}
	if (geo.useIndexBuffers)
	{

		uint32_t numVertex = geo.vertexAttrib.vertices.size() / 3;
		Geometry& geometry = geo;
		geometry.numVertex = numVertex;
		//����vertex buffer
		geometry.vertexBuffer = CreateVertexBuffer(nullptr, vertexAttributeInputStride * numVertex);
		for (uint32_t vertexId = 0; vertexId < numVertex; vertexId++)
		{
			//��䶥��λ������
			FillBuffer(geometry.vertexBuffer, vertexId * vertexAttributeInputStride, 3 * sizeof(float), (const char*)(geo.vertexAttrib.vertices.data() + vertexId * 3));



		}

		geometry.indexBuffers.resize(geo.shapes.size());
		std::vector<uint32_t> indicesData;
		geometry.numIndexPerZone.resize(geometry.indexBuffers.size());
		//手动计算法线
		std::vector<glm::vec3> vertexNormals(numVertex, glm::vec3(0));
		std::map<uint32_t, std::set<uint32_t>> vertexNormalIds;

		//纹理坐标数据待定

		for (uint32_t zoneId = 0; zoneId < geometry.indexBuffers.size(); zoneId++)
		{

			for (uint32_t cellId = 0; cellId < geo.shapes[zoneId].mesh.num_face_vertices.size(); cellId++)
			{
				if (geo.shapes[zoneId].mesh.num_face_vertices[cellId] != 3)
				{
					LogFunc(0);//������������ε�ģ�;�ֱ�ӱ���
				}
			}
			indicesData.resize(geo.shapes[zoneId].mesh.num_face_vertices.size() * 3);
			for (uint32_t i = 0; i < geo.shapes[zoneId].mesh.indices.size(); i++)
			{
				indicesData[i] = geo.shapes[zoneId].mesh.indices[i].vertex_index;//��Ŷ�������
				const auto& normalIndex = geo.shapes[zoneId].mesh.indices[i].normal_index;
				//�����������
				//填充法线
				if (!geo.vertexAttrib.normals.empty())
				{
					glm::vec3 curNormal = glm::vec3(geo.vertexAttrib.normals[normalIndex * 3], geo.vertexAttrib.normals[normalIndex * 3 + 1], geo.vertexAttrib.normals[normalIndex * 3 + 2]);
					curNormal = glm::normalize(curNormal);
					if (!vertexNormalIds[indicesData[i]].contains(normalIndex))//如果这是一个新的法线则加入计算
					{
						vertexNormalIds[indicesData[i]].insert(normalIndex);
						vertexNormals[indicesData[i]] += curNormal;
					}


					//FillBuffer(geometry.vertexBuffer, indicesData[i] * vertexAttributeInputStride + 3 * sizeof(float), 3 * sizeof(float), (const char*)(geo.vertexAttrib.normals.data() + normalIndex * 3));
				}
			}

			geometry.indexBuffers[zoneId] = CreateIndexBuffer((const char*)indicesData.data(), indicesData.size() * sizeof(uint32_t));
			geometry.numIndexPerZone[zoneId] = indicesData.size();

		}


		for (uint32_t v = 0; v < numVertex; v++)
		{
			glm::vec3 curV = vertexNormals[v];
			//归一化
			curV = glm::normalize(curV);
			//填充法向量
			FillBuffer(geometry.vertexBuffer, v * vertexAttributeInputStride + 3 * sizeof(float), 3 * sizeof(float), (const char*)(&curV));

		}





	}
	else {

		geo.shapeVertexBuffers.resize(geo.shapes.size());
		geo.numIndexPerZone.resize(geo.shapeVertexBuffers.size());
		std::vector<float> curVertexData;
		for (uint32_t zoneId = 0; zoneId < geo.shapeVertexBuffers.size(); zoneId++)
		{

			for (uint32_t cellId = 0; cellId < geo.shapes[zoneId].mesh.num_face_vertices.size(); cellId++)
			{
				if (geo.shapes[zoneId].mesh.num_face_vertices[cellId] != 3)
				{
					LogFunc(0);//������������ε�ģ�;�ֱ�ӱ���
				}
			}
			//当前shape的所有片元的点数据
			uint32_t vertexAttributeInputFloatStride = vertexAttributeInputStride / sizeof(float);
			curVertexData.resize(geo.shapes[zoneId].mesh.indices.size() * vertexAttributeInputFloatStride);
			for (uint32_t i = 0; i < geo.shapes[zoneId].mesh.indices.size(); i++)
			{
				const auto & vertexIndex = geo.shapes[zoneId].mesh.indices[i].vertex_index;//��Ŷ�������
				const auto& normalIndex = geo.shapes[zoneId].mesh.indices[i].normal_index;
				const auto& texCoordIndex = geo.shapes[zoneId].mesh.indices[i].texcoord_index;
				//�����������
				//填充顶点位置
				if (!geo.vertexAttrib.vertices.empty())
				{
					glm::vec3 curVertex = glm::vec3(geo.vertexAttrib.vertices[vertexIndex * 3], geo.vertexAttrib.vertices[vertexIndex * 3 + 1], geo.vertexAttrib.vertices[vertexIndex * 3 + 2]);
					float* curVertexOffset = curVertexData.data() + i * vertexAttributeInputFloatStride;
					std::memcpy(curVertexOffset, &curVertex, sizeof(float) * 3);
				}
				
				//填充法线
				if (!geo.vertexAttrib.normals.empty())
				{
					glm::vec3 curNormal = glm::vec3(geo.vertexAttrib.normals[normalIndex * 3], geo.vertexAttrib.normals[normalIndex * 3 + 1], geo.vertexAttrib.normals[normalIndex * 3 + 2]);
					curNormal = glm::normalize(curNormal);
					float* curVertexOffset = curVertexData.data() + i * vertexAttributeInputFloatStride + 3;
					std::memcpy(curVertexOffset, &curNormal, sizeof(float) * 3);
				}
				//填充纹理坐标
				if (!geo.vertexAttrib.texcoords.empty())
				{
					glm::vec2 curTexCoord = glm::vec2(geo.vertexAttrib.texcoords[texCoordIndex*2], geo.vertexAttrib.texcoords[texCoordIndex * 2 + 1]);
					float* curVertexOffset = curVertexData.data() + i * vertexAttributeInputFloatStride + 9;
					std::memcpy(curVertexOffset, &curTexCoord, sizeof(float) * 2);

				}
			}

			geo.shapeVertexBuffers[zoneId] = CreateIndexBuffer((const char*)curVertexData.data(), curVertexData.size() * sizeof(float));
			geo.numIndexPerZone[zoneId] = geo.shapes[zoneId].mesh.indices.size();

		}





	}





}

void ExampleBase::InitTextureResources()
{
	//根据texture infos创建texture并更新
	for (auto& textureInfo : textureBindInfos)
	{
		Texture texture;
		texture = CreateTexture(textureInfo.second);
		textures[textureInfo.first] = texture;
		
	}

}

void ExampleBase::InitUniformBufferResources()
{
	for (const auto& bufferBindInfo : bufferBindInfos)
	{
		buffers[bufferBindInfo.first] = CreateShaderAccessBuffer(nullptr, bufferBindInfo.second.size);
		//BindUniformBuffer(uniformBuffers[unifomBufferInfo.first], unifomBufferInfo.second.pipeId, unifomBufferInfo.second.setId, unifomBufferInfo.second.binding, unifomBufferInfo.second.elementId);
	}

}

void ExampleBase::InitCompute()
{
	if (computeDesc.valid)
	{
		//解析shader中的descriptor信息
		TransferGLSLFileToSPIRVFileAndRead(computeDesc.computeShaderPath, computePipelineInfos.computeShaderResourceInfo.spirvCode);
		ParseSPIRVShaderResourceInfo(computePipelineInfos.computeShaderResourceInfo.spirvCode, computePipelineInfos.computeShaderResourceInfo);

	

		InitComputePipeline();



	}
}

void ExampleBase::InitComputePipeline()
{
	//创建描述符集
	std::map<VkDescriptorType, uint32_t> needNumDescriptor;
	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setBindins;
	VkDescriptorSetLayoutBinding tmpBinding;
	tmpBinding.pImmutableSamplers = nullptr;
	tmpBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	for (const auto& setBinding : computePipelineInfos.computeShaderResourceInfo.descriptorSetBindings)
	{
		for (const auto& binding : setBinding.second)
		{
			needNumDescriptor[binding.descriptorType] += binding.numDescriptor;
			tmpBinding.binding = binding.binding;
			tmpBinding.descriptorCount = binding.numDescriptor;
			tmpBinding.descriptorType = binding.descriptorType;
			setBindins[binding.setId].push_back(tmpBinding);
		}
		
	}



	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& needDescriptor : needNumDescriptor)
	{
		VkDescriptorPoolSize poolSize;
		poolSize.type = needDescriptor.first;
		poolSize.descriptorCount = needDescriptor.second;
		poolSizes.push_back(poolSize);
	}
	auto descriptorPool = CreateDescriptorPool(device, 0, computePipelineInfos.computeShaderResourceInfo.descriptorSetBindings.size() + 1, poolSizes);
	computePipelineInfos.descriptorPool = descriptorPool;


	//����descriptor set
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	for (const auto& setAndBindingInfos : setBindins)
	{
		//����descriptor set layout
		auto descriptorSetLayout = CreateDescriptorSetLayout(device, 0, setAndBindingInfos.second);
		computePipelineInfos.descriptorSetInfos[setAndBindingInfos.first].setLayout = descriptorSetLayout;
		auto descriptorSet = AllocateDescriptorSet(device,computePipelineInfos.descriptorPool, { descriptorSetLayout });
		computePipelineInfos.descriptorSetInfos[setAndBindingInfos.first].descriptorSet = descriptorSet;
		descriptorSetLayouts.push_back(descriptorSetLayout);
		computePipelineInfos.descriptorSetInfos[setAndBindingInfos.first].bindings = setBindins[setAndBindingInfos.first];
	}

	//����pipeline layout
	auto pipelineLayout = CreatePipelineLayout(device, 0, descriptorSetLayouts, {});
	computePipelineInfos.pipelineLayout = pipelineLayout;


	
	auto shaderModule = CreateShaderModule(device, 0, computePipelineInfos.computeShaderResourceInfo.spirvCode);
	computePipelineInfos.computeShaderStage.module = shaderModule;
	computePipelineInfos.computeShaderStage.pName = computePipelineInfos.computeShaderResourceInfo.entryName.c_str();
	computePipelineInfos.pipeline = CreateComputePipeline(device, 0, computePipelineInfos.computeShaderStage, computePipelineInfos.pipelineLayout);

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
		if (type.array.empty())
		{
			bindingInfo.numDescriptor = 1;
		}
		else {
			bindingInfo.numDescriptor = type.array[0];
		}
		//bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
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
		if (type.array.empty())
		{
			bindingInfo.numDescriptor = 1;
		}
		else {
			bindingInfo.numDescriptor = type.array[0];
		}
		//bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dstCacheShaderResource.descriptorSetBindings[bindingInfo.setId].push_back(bindingInfo);
	}


	//����store image��Ϣ
	for (uint32_t i = 0; i < resources.storage_images.size(); i++)
	{
		auto& storeImage = resources.storage_images[i];
		auto type = shaderCompiler.get_type(storeImage.base_type_id);
		bindingInfo.setId = shaderCompiler.get_decoration(storeImage.id, spv::Decoration::DecorationDescriptorSet);
		bindingInfo.binding = shaderCompiler.get_decoration(storeImage.id, spv::DecorationBinding);
		bindingInfo.name = storeImage.name;
		if (type.array.empty())
		{
			bindingInfo.numDescriptor = 1;
		}
		else {
			bindingInfo.numDescriptor = type.array[0];
		}
		//bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		dstCacheShaderResource.descriptorSetBindings[bindingInfo.setId].push_back(bindingInfo);
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
	buffer.memory = AllocateMemory(device, memRequirements.size, memtypeIndex);
	BindMemoryToBuffer(device, buffer.memory, buffer.buffer, 0);
	if (memoryPropties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		buffer.hostMapPointer = MapMemory(device, buffer.memory, 0, size, 0);
		if (buf)
		{
			std::memcpy(buffer.hostMapPointer, buf, size);
		}
		else {
			std::memset(buffer.hostMapPointer, 0, size);
		}
	}


	return buffer;
}




uint32_t ExampleBase::GetNextPresentImageIndex(VkSemaphore sigValidSemaphore)
{
	uint32_t nextImageIndex = VulkanAPI::GetNextValidSwapchainImageIndex(device, swapchain, sigValidSemaphore, nullptr);
	return nextImageIndex;
}

VkDescriptorType ExampleBase::GetDescriptorType(DescriptorSetInfo& descriptorSetInfo, uint32_t binding)
{

	for (const auto& setbinding : descriptorSetInfo.bindings)
	{
		if (setbinding.binding == binding)
		{
			return setbinding.descriptorType;
		}
	}

	ASSERT(0);
	return VkDescriptorType();
}

void ExampleBase::BindTexture(const Texture& texture, VkDescriptorSet set, uint32_t binding, uint32_t elemenId, VkDescriptorType descriptorType)
{
	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = texture.sampler;
	descriptorImageInfo.imageLayout = texture.image.currentLayout;
	descriptorImageInfo.imageView = texture.image.imageView;
	
	UpdateDescriptorSetBindingResources(device, set, binding, elemenId, 1, descriptorType, { descriptorImageInfo }, {}, {});


}

void ExampleBase::BindBuffer(const Buffer& uniformBuffer, VkDescriptorSet set, uint32_t binding, uint32_t elemenId, VkDescriptorType descriptorType)
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = uniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = uniformBuffer.size;
	UpdateDescriptorSetBindingResources(device, set, binding, elemenId, 1, descriptorType, {  }, { descriptorBufferInfo }, {});

}


void ExampleBase::WaitIdle()
{
	WaitQueueIdle(graphicQueue);

}

void ExampleBase::WaitAllFence(const std::vector<VkFence>& fences)
{
	auto res = WaitFence(device, fences, true);
	if (res != VK_SUCCESS)
	{
		auto status = GetFenceStatus(device, fences[0]);
		ASSERT(0);
	}
}

void ExampleBase::ResetAllFence(const std::vector<VkFence>& fences)
{
	ResetFences(device, fences);
}

//void ExampleBase::CreateFences(uint32_t numFences)
//{
//	fences.resize(numFences);
//	for (uint32_t i = 0; i < numFences; i++)
//	{
//		fences[i] = CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT);
//	}
//}

void ExampleBase::CreateSemaphores(uint32_t numSemaphores)
{
	semaphores.resize(numSemaphores);
	for (uint32_t i = 0; i < numSemaphores; i++)
	{
		semaphores[i] = CreateSemaphore(device, 0);
	}


}

void ExampleBase::SetSupassDescription(VkSubpassDescription& subpassDesc, VkSubpassDescriptionFlags flags, VkPipelineBindPoint pipelineBindPoint, const std::vector<VkAttachmentReference>& inputAttachmentRefs, const std::vector<VkAttachmentReference>& colorAttachmentRefs, const VkAttachmentReference* resolveAttachments, const VkAttachmentReference* depthStencilAttachment, const std::vector<uint32_t>& preserveAttachments)
{
	subpassDesc.flags = flags;
	subpassDesc.pipelineBindPoint = pipelineBindPoint;
	subpassDesc.inputAttachmentCount = inputAttachmentRefs.size();
	if (subpassDesc.inputAttachmentCount != 0)
	{
		subpassDesc.pInputAttachments = inputAttachmentRefs.data();
	}
	subpassDesc.colorAttachmentCount = colorAttachmentRefs.size();
	if (subpassDesc.colorAttachmentCount != 0)
	{
		subpassDesc.pColorAttachments = colorAttachmentRefs.data();
	}
	subpassDesc.pDepthStencilAttachment = depthStencilAttachment;
	subpassDesc.pResolveAttachments = resolveAttachments;

	subpassDesc.preserveAttachmentCount = preserveAttachments.size();
	if (subpassDesc.preserveAttachmentCount != 0)
	{
		subpassDesc.pPreserveAttachments = preserveAttachments.data();
	}
}

