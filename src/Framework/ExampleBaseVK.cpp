#include "ExampleBaseVK.h"
#include "Utils/ImageFileTool.h"



#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/Public/resource_limits_c.h"

#include <shaderc/shaderc.h>
#include <filesystem>
#include <fstream>


using namespace VulkanAPI;

ExampleBaseVK::~ExampleBaseVK()
{
	Clear();
}


void ExampleBaseVK::BindTexture(const std::string& textureName)
{
	//绑定texture
	ASSERT(textures.find(textureName) != textures.end());
	auto& textureInfo = textureBindInfos[textureName];
	if (!textureInfo.compute)
	{

		if (!textureInfo.rayTracing)
		{
			auto& graphcisPipelineInfos = renderPassInfos[textureInfo.passId].graphcisPipelineInfos;
			auto descriptorType = GetDescriptorType(graphcisPipelineInfos[textureInfo.pipeId].descriptorSetInfos[textureInfo.setId], textureInfo.binding);
			BindTexture(textures[textureName], graphcisPipelineInfos[textureInfo.pipeId].descriptorSetInfos[textureInfo.setId].descriptorSet, textureInfo.binding, textureInfo.elementId, descriptorType);
		}
		else {
			auto& rayTracingPipelineInfos = rayTracingPipelinesInfos[textureInfo.passId];
			auto descriptorType = GetDescriptorType(rayTracingPipelineInfos.descriptorSetInfos[textureInfo.setId], textureInfo.binding);
			BindTexture(textures[textureName], rayTracingPipelineInfos.descriptorSetInfos[textureInfo.setId].descriptorSet, textureInfo.binding, textureInfo.elementId, descriptorType);

		}
	
	}
	else {
		auto& computePipelineInfos = computePipelinesInfos[textureInfo.passId];
		auto descriptorType = GetDescriptorType(computePipelineInfos.descriptorSetInfos[textureInfo.setId], textureInfo.binding);
		BindTexture(textures[textureName], computePipelineInfos.descriptorSetInfos[textureInfo.setId].descriptorSet, textureInfo.binding, textureInfo.elementId, descriptorType);
	}
	
}

void ExampleBaseVK::BindBuffer(const std::string& bufferName)
{
	ASSERT(buffers.find(bufferName) != buffers.end());
	auto& bufferInfo = bufferBindInfos[bufferName];
	if (!bufferInfo.compute)
	{
		auto& graphcisPipelineInfos = renderPassInfos[bufferInfo.passId].graphcisPipelineInfos;
		auto descriptorType = GetDescriptorType(graphcisPipelineInfos[bufferInfo.pipeId].descriptorSetInfos[bufferInfo.setId], bufferInfo.binding);
		BindBuffer(buffers[bufferName], graphcisPipelineInfos[bufferInfo.pipeId].descriptorSetInfos[bufferInfo.setId].descriptorSet, bufferInfo.binding, bufferInfo.elementId, descriptorType);
	}
	else {
		auto& computePipelineInfos = computePipelinesInfos[bufferInfo.passId];
		auto descriptorType = GetDescriptorType(computePipelineInfos.descriptorSetInfos[bufferInfo.setId], bufferInfo.binding);
		BindBuffer(buffers[bufferName], computePipelineInfos.descriptorSetInfos[bufferInfo.setId].descriptorSet, bufferInfo.binding, bufferInfo.elementId, descriptorType);
	}
	
}

void ExampleBaseVK::BindAccelerationStructure(const std::string& accelerationStructureName)
{
	ASSERT(accelerationStructureBindInfos.find(accelerationStructureName) != accelerationStructureBindInfos.end());

	auto& bindInfo = accelerationStructureBindInfos[accelerationStructureName];
	auto& rayTracingPipelineInfo = rayTracingPipelinesInfos[bindInfo.pipeId];
	auto descriptorType = GetDescriptorType(rayTracingPipelineInfo.descriptorSetInfos[bindInfo.setId], bindInfo.binding);

	BindAccelerationStructure(bindInfo.geometryIndex, rayTracingPipelineInfo.descriptorSetInfos[bindInfo.setId].descriptorSet, bindInfo.binding, bindInfo.elementId, descriptorType);

}

void ExampleBaseVK::ReInitGeometryResources(Geometry& geo)
{
	if (rayTracingPipelinsDesc.valid)
	{
		DestroyAccelerationStructureKHR(device,geo.accelerationStructureKHR);
	}


	DestroyBuffer(geo.vertexBuffer);
	for (uint32_t zoneId = 0; zoneId < geo.indexBuffers.size(); zoneId++)
	{
		DestroyBuffer(geo.indexBuffers[zoneId]);
	}
	for (uint32_t zoneId = 0; zoneId < geo.shapeVertexBuffers.size(); zoneId++)
	{
		DestroyBuffer(geo.shapeVertexBuffers[zoneId]);
	}
	//for (uint32_t zoneId = 0; zoneId < geo.shapeDynamicVertexBuffers.size(); zoneId++)
	//{
	//	DestroyBuffer(geo.shapeDynamicVertexBuffers[zoneId]);
	//}

	InitGeometryResources(geo);


}

void ExampleBaseVK::ResizeBuffer(Buffer& buffer, VkDeviceSize newByteSize)
{
	DestroyBuffer(buffer);
	buffer = CreateBuffer(buffer.usage, nullptr, newByteSize, buffer.memoryPropties);
}



void ExampleBaseVK::Init()
{
	if (initFlag)
	{
		return;
	}
	initFlag = true;
	Initialize();
	InitContex();
	InitRenderPasses();
	InitQueryPool();
	InitSyncObject();
	InitRecources();
	InitComputeInfo();
	InitCompute();
	InitRayTracing();
}



void ExampleBaseVK::ParseShaderFiles(RenderPassInfo& renderPassInfo)
{
	auto& graphcisPipelineInfos = renderPassInfo.graphcisPipelineInfos;
	auto& subpassInfo = renderPassInfo.subpassInfo;
	graphcisPipelineInfos.resize(subpassInfo.subpassDescs.size());
	std::vector<char> tmpCode;
	VkShaderStageFlagBits curShaderStage = VK_SHADER_STAGE_VERTEX_BIT;

	
	for (uint32_t i = 0; i < subpassInfo.subpassDescs.size(); i++)
	{
		const auto& shaderPaths = subpassInfo.subpassDescs[i].pipelinesShaderCodePaths;
		auto& pipelineShaderResourceInfo = graphcisPipelineInfos[i].pipelineShaderResourceInfo;
		renderPassInfo.isMeshSubpass[i] = false;
		if (!shaderPaths.meshShaderPath.empty())
		{
			renderPassInfo.isMeshSubpass[i] = true;
		}
		
		bool& inputAttachmentEnable = subpassInfo.subpassDescs[i].enableInputAttachment;

		//mesh
		curShaderStage = VK_SHADER_STAGE_MESH_BIT_EXT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.meshShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.meshShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);

		//task
		curShaderStage = VK_SHADER_STAGE_TASK_BIT_EXT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.taskShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.taskShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);



		//vertex
		curShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.vertexShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.vertexShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		
		ParseSPIRVShaderInputAttribute(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.inputAttributesInfo);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);

		//tessellation control
		curShaderStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.tessellationControlShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.tessellationControlShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);


		//tessellation evaluation
		curShaderStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.tessellationEvaluationShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.tessellationEvaluationShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);

		
		//geom
		curShaderStage = VK_SHADER_STAGE_GEOMETRY_BIT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.geometryShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.geometryShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);
				
		//frag
		curShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].shaderFilePath = shaderPaths.fragmentShaderPath;
		TransferGLSLFileToSPIRVFileAndRead(shaderPaths.fragmentShaderPath, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode);
		ParseSPIRVShaderResourceInfo(pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage].spirvCode, pipelineShaderResourceInfo.shaderResourceInfos[curShaderStage]);




	}



}

void ExampleBaseVK::InitContex()
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
	std::vector<const char*> enableInstanceLayers{};
	for (uint32_t i = 0; i < supportLayers.size(); i++)
	{
		if (strcmp(supportLayers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0)
		{
			enableInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
			break;
		}
	}
	
	if (!supportValidateLayer)
	{
		LogFunc(0);
		LogInfo("validation layer is not surpported");
	}

	//auto 

	//����instance
	instance = CreateInstance(enableInstanceLayers, wantExtensions);//这里只有instance开启了debug util
	//����surface
	surface = CreateWin32Surface(instance, window);
	debugUtilMessager = CreateDebugInfoMessager(instance);
	PickValidPhysicalDevice();
	CheckCandidateTextureFormatSupport();
	//ASSERT(physicalDeviceFeatures.geometryShader == VK_TRUE);//检查是否支持几何着色器
	//ASSERT(physicalDeviceFeatures.tessellationShader == VK_TRUE);//检查是否支持曲面细分着色器
	//ASSERT(physicalDeviceMeshShaderFeaturesEXT.meshShader == VK_TRUE);//检查是否支持mesh着色器
	
	std::vector<const char*> enableExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	void* extendInfoP = nullptr;
	VkPhysicalDeviceFeatures* enableFeature = &physicalDeviceFeatures;
	if (enableMeshShaderEXT)
	{
		extendInfoP = &physicalDeviceFeatures2;
		physicalDeviceFeatures2.pNext = &physicalDeviceMeshShaderFeaturesEXT;
		enableFeature = nullptr; 
		enableExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
		enableExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
	}
	if (rayTracingPipelinsDesc.valid)
	{
		extendInfoP = &physicalDeviceFeatures2;
		physicalDeviceAccelerationStructureFeaturesKHR.pNext = physicalDeviceFeatures2.pNext;
		physicalDeviceFeatures2.pNext = &physicalDeviceRayTracingPipelineFeaturesKHR;
		enableFeature = nullptr;
		enableExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		enableExtensions.push_back(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME); 
		enableExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		enableExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		enableExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		enableExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		
		//enableExtensions.push_back("VK_KHR_portability_subset");

	}
	void* pNext = physicalDeviceFeatures2.pNext;
	while (pNext != nullptr) {
		// 获取结构体的类型
		const VkBaseInStructure* base = reinterpret_cast<const VkBaseInStructure*>(pNext);
		std::cout << "Structure type: ";
		// 移动到下一个结构体
		pNext = (void*)base->pNext;
	}

	ASSERT(CheckExtensionSupport(physicalDevice, enableExtensions));

	
	device = CreateDevice(physicalDevice, { {queueFamilyIndex,{1}} }, {}, enableExtensions, enableFeature, extendInfoP);//device之开启了swapchain拓展
	graphicQueue = GetQueue(device, queueFamilyIndex, 0);

	if (enableMeshShaderEXT || rayTracingPipelinsDesc.valid)
	{
		LoadExtensionAPIs(device);

	}

	//����command pool
	commandPool = CreateCommandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndex);
	InitCommandList();

	//����swapchain
	swapchain = CreateSwapchain(device, surface, swapchainImageFormat, colorSpace, VkExtent2D{ .width = windowWidth,.height = windowHeight }, 1, 1, 2, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, { queueFamilyIndex }, swapchainPresentMode);


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
		swapchainImages[i].format = swapchainImageFormat;
		//ת��swapchain image��image layout
		TransferWholeImageLayout(swapchainImages[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);//�ȴ�������Ⱦ���


	}


	













}

void ExampleBaseVK::InitAttanchmentDesc(RenderPassInfo& renderPassInfo)
{
	//����color attachment����Դ
	ASSERT(renderPassInfo.renderTargets.colorAttachments.size() >= 1);
	for (uint32_t i = 0;i < renderPassInfo.renderTargets.colorAttachments.size();i++)
	{

		auto& colorAttachment = renderPassInfo.renderTargets.colorAttachments[i].attachmentDesc;
		auto& colorImage = renderPassInfo.renderTargets.colorAttachments[i].attachmentImage;

		if (colorAttachment.format != RenderTargets::colorFormat)
		{
			ASSERT(CheckOptimalFormatFeatureSupport(physicalDevice, colorAttachment.format, RenderTargets::colorAttachmentFormatFeatures));


		}

		VkImageUsageFlags colorAttachmentImageUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (renderPassInfo.renderTargets.enaleInputAttachment)
		{
			colorAttachmentImageUsages |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}


		colorImage = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, colorAttachment.format, colorImage.extent.width, colorImage.extent.height, 1, 1, 1, colorAttachmentImageUsages, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkComponentMapping{}, VK_IMAGE_TILING_OPTIMAL);
		//TransferWholeImageLayout(colorImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		//renderPassInfo.renderTargets.colorAttachment.clearValue = VkClearValue{ 0,0,0,1 };



		colorAttachment.flags = 0;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = colorImage.currentLayout;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorImage.currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;




	}





	auto& depthImage = renderPassInfo.renderTargets.depthAttachment.attachmentImage;
	auto& depthAttachment = renderPassInfo.renderTargets.depthAttachment.attachmentDesc;

	if (depthAttachment.format != RenderTargets::depthFormat)
	{

		ASSERT(CheckOptimalFormatFeatureSupport(physicalDevice, depthAttachment.format, RenderTargets::depthAttachmentFormatFeatures));


	}

	depthImage = CreateImage(VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, depthAttachment.format, depthImage.extent.width, depthImage.extent.height, 1, 1, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkComponentMapping{}, VK_IMAGE_TILING_OPTIMAL);
	//TransferWholeImageLayout(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	//renderPassInfo.renderTargets.depthAttachment.clearValue = VkClearValue{ 1.0,0 };




	depthAttachment.flags = 0;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = renderPassInfo.renderTargets.depthAttachment.attachmentImage.currentLayout;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	renderPassInfo.renderTargets.depthAttachment.attachmentImage.currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;





}

void ExampleBaseVK::InitRenderPass(RenderPassInfo& renderPassInfo)
{
	//创建render pass
	std::vector<VkSubpassDescription> subpassDescriptions;
	for (uint32_t i = 0; i < renderPassInfo.subpassInfo.subpassDescs.size(); i++)
	{
		subpassDescriptions.push_back(renderPassInfo.subpassInfo.subpassDescs[i].subpassDescription);
	}
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	for (uint32_t i = 0; i < renderPassInfo.renderTargets.colorAttachments.size(); i++)
	{
		attachmentDescriptions.push_back(renderPassInfo.renderTargets.colorAttachments[i].attachmentDesc);

	}
	attachmentDescriptions.push_back(renderPassInfo.renderTargets.depthAttachment.attachmentDesc);

	renderPassInfo.renderPass = CreateRenderPass(device, 0, attachmentDescriptions, subpassDescriptions, renderPassInfo.subpassInfo.subpassDepends);


}

void ExampleBaseVK::InitRenderPasses()
{
	//分析和创建RenderPass对应的shader信息
	for (uint32_t i = 0; i < renderPassInfos.size(); i++)
	{
		ParseShaderFiles(renderPassInfos[i]);
		InitAttanchmentDesc(renderPassInfos[i]);
		InitRenderPass(renderPassInfos[i]);
		InitFrameBuffer(renderPassInfos[i]);
		InitGraphicPipelines(renderPassInfos[i]);

		if (renderPassInfos[i].renderTargets.enaleInputAttachment)
		{
			BindInputAttachment(renderPassInfos[i]);
		}

	}

	







}

void ExampleBaseVK::InitFrameBuffer(RenderPassInfo& renderPassInfo)
{
	uint32_t width = renderPassInfo.renderTargets.width;
	uint32_t height = renderPassInfo.renderTargets.height;

	std::vector<VkImageView> attachmentsImagViews;
	for (uint32_t i = 0; i < renderPassInfo.renderTargets.colorAttachments.size(); i++)
	{
		attachmentsImagViews.push_back(renderPassInfo.renderTargets.colorAttachments[i].attachmentImage.imageView);
	}
	attachmentsImagViews.push_back(renderPassInfo.renderTargets.depthAttachment.attachmentImage.imageView);
	renderPassInfo.frameBuffer = CreateFrameBuffer(device, 0, renderPassInfo.renderPass, attachmentsImagViews, width, height,1);

}

void ExampleBaseVK::InitSyncObject()
{
	//CreateFence(device,numFences);
	CreateSemaphores(numSemaphores);
}

void ExampleBaseVK::InitDefaultGraphicSubpassInfo(ShaderCodePaths subpassShaderCodePaths)
{
	renderPassInfos.resize(1);
	auto& subpassInfo = renderPassInfos[0].subpassInfo;
	auto& renderTargets = renderPassInfos[0].renderTargets;

	subpassInfo.subpassDescs.resize(1);
	subpassInfo.subpassDescs[0].subpassPipelineStates.Init(windowWidth, windowHeight);
	subpassInfo.subpassDescs[0].pipelinesShaderCodePaths = subpassShaderCodePaths;
	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.subpassDescription.flags = 0;
	subpassDesc1.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.subpassDescription.inputAttachmentCount = 0;
	subpassDesc1.subpassDescription.pInputAttachments = nullptr;
	subpassDesc1.subpassDescription.colorAttachmentCount = renderTargets.colorAttachments.size();
	subpassDesc1.subpassDescription.pColorAttachments = renderTargets.colorRefs.data();
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


int32_t ExampleBaseVK::GetSuitableQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags wantQueueFlags, bool needSupportPresent, uint32_t wantNumQueue)
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

void ExampleBaseVK::PickValidPhysicalDevice()
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
			//检查默认深度附件格式支持

			if (!CheckOptimalFormatFeatureSupport(physicalDevices[i], RenderTargets::depthFormat, RenderTargets::depthAttachmentFormatFeatures)) {
				continue;
			}

			//检查默认颜色附件格式支持
			if (!CheckOptimalFormatFeatureSupport(physicalDevices[i], RenderTargets::colorFormat, RenderTargets::colorAttachmentFormatFeatures)) {
				//FindFormat(physicalDevices[i], RenderTargets::colorAttachmentFormatFeatures);
				continue;
			}


			//纹理的默认格式是VK_FORMAT_R8G8B8A8_UNORM,所以这里检查一下是否支持
			if (!CheckLinearFormatFeatureSupport(physicalDevices[i], TextureBindInfo::defaultTextureFormat, TextureBindInfo::textureFormatFeatures)) {
				continue;
			}





			auto surfaceFormatsCapabilities = GetSurfaceFormats(physicalDevices[i], surface);
			//���֧��color��format
			bool surpportColorFormat = false;
			for (auto& surfaceFormatCapabilities : surfaceFormatsCapabilities)
			{
				if (surfaceFormatCapabilities.format == swapchainImageFormat )
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
			

			//检查特性

			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.pNext = VK_NULL_HANDLE;
			if (enableMeshShaderEXT)//如果使用feature2
			{
				physicalDeviceFeatures2.pNext = &physicalDeviceMeshShaderFeaturesEXT;
				physicalDeviceMeshShaderFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
				physicalDeviceMeshShaderFeaturesEXT.pNext = &maintenance4Feature;
		

				maintenance4Feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
				maintenance4Feature.pNext = VK_NULL_HANDLE;

				GetPhysicalDeviceFeatures2(physicalDevices[i], physicalDeviceFeatures2);
				if (physicalDeviceMeshShaderFeaturesEXT.meshShader != VK_TRUE || physicalDeviceMeshShaderFeaturesEXT.taskShader != VK_TRUE) {
					continue;//检查是否支持mesh着色器以及task着色器
				}
				if (physicalDeviceFeatures2.features.geometryShader != VK_TRUE || physicalDeviceFeatures2.features.tessellationShader != VK_TRUE) {
					continue;	//检查是否支持几何着色器以及细分着色器
				}

			}
			else {
				physicalDeviceFeatures = GetPhysicalDeviceFeatures(physicalDevices[i]);

				if (physicalDeviceFeatures.geometryShader != VK_TRUE || physicalDeviceFeatures.tessellationShader != VK_TRUE) {
					continue;	//检查是否支持几何着色器以及细分着色器
				}
			
			}



			if (rayTracingPipelinsDesc.valid)
			{
				physicalDeviceFeatures2.pNext = &physicalDeviceRayTracingPipelineFeaturesKHR;
				physicalDeviceRayTracingPipelineFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
				physicalDeviceRayTracingPipelineFeaturesKHR.pNext = &physicalDeviceVulkanMemoryModelFeatures;
				

				physicalDeviceVulkanMemoryModelFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES;
				physicalDeviceVulkanMemoryModelFeatures.pNext = &physicalDeviceBufferDeviceAddressFeaturesKHR;

				physicalDeviceBufferDeviceAddressFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
				physicalDeviceBufferDeviceAddressFeaturesKHR.pNext = &physicalDeviceAccelerationStructureFeaturesKHR;


				physicalDeviceAccelerationStructureFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
				physicalDeviceAccelerationStructureFeaturesKHR.pNext = VK_NULL_HANDLE;


				GetPhysicalDeviceFeatures2(physicalDevices[i], physicalDeviceFeatures2);
				if (!physicalDeviceRayTracingPipelineFeaturesKHR.rayTracingPipeline ||
					!physicalDeviceVulkanMemoryModelFeatures.vulkanMemoryModel ||
					!physicalDeviceBufferDeviceAddressFeaturesKHR.bufferDeviceAddress ||
					!physicalDeviceAccelerationStructureFeaturesKHR.accelerationStructure 

					) {
					continue;

				}
			}


			queueFamilyIndex = familyIndex;
			physicalDevice = physicalDevices[i];
			physicalDeviceProps = physicalDeviceProperties[i];

			physicalDeviceFeatures2.pNext = VK_NULL_HANDLE;
			return;
		}


	}

	ASSERT(physicalDevice);
	LogFunc(0);

}

void ExampleBaseVK::CheckCandidateTextureFormatSupport()
{
	uint64_t textureWantFormatFeature = TextureBindInfo::defaultTextureFormat;
	for (const auto& candidatedFormat : candidatedTextureFormats)
	{
		auto textureFormatProps = GetFormatPropetirs(physicalDevice, candidatedFormat);

		if ((textureFormatProps.linearTilingFeatures & textureWantFormatFeature) == textureWantFormatFeature)
		{
			std::cout << "The texture format is supported: " << VkFormatToInfo[candidatedFormat].name << std::endl;
		}
		else {
			std::cout << "The texture format is not supported: " << VkFormatToInfo[candidatedFormat].name << std::endl;
		}
	}



}

int32_t ExampleBaseVK::GetMemoryTypeIndex(uint32_t  wantMemoryTypeBits, VkMemoryPropertyFlags wantMemoryFlags)
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

Image ExampleBaseVK::CreateImage(VkImageType imageType,VkImageViewType viewType,VkFormat format,uint32_t width,uint32_t height, uint32_t depth,uint32_t numMip,uint32_t numLayer,VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags memoryProperies, VkComponentMapping viewMapping, VkImageTiling tiling,VkSampleCountFlagBits sample, VkImageLayout layout)
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
	image.memory = AllocateMemory(device, imageMemRequiredments.size, memtypeIndex,0);
	image.totalMemorySize = imageMemRequiredments.size;
	BindMemoryToImage(device, image.memory, image.image, 0);
	if (memoryProperies & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		image.hostMapPointer = MapMemory(device, image.memory, 0, imageMemRequiredments.size, 0);
	}
	image.imageView = CreateImageView(device, 0, image.image,usage, viewType, format, viewMapping, VkImageSubresourceRange{ .aspectMask = aspect,.baseMipLevel = 0,.levelCount = image.numMip ,.baseArrayLayer = 0,.layerCount = image.numLayer });

	return image;
}

void ExampleBaseVK::FillImageFromDataSource(Image& image, TextureBindInfo& textureBindInfo)
{
	uint32_t texdelByteSize = VkFormatToInfo[image.format].totalBytesPerPixel;
	if (textureBindInfo.viewType == VK_IMAGE_VIEW_TYPE_3D)
	{
		auto& sublayout = image.layerMipLayouts[0][0];
		uint32_t depth = textureBindInfo.textureDataSources.size();
		for (uint32_t d = 0; d < depth; d++)
		{

			uint32_t height = textureBindInfo.textureDataSources[d].height;
			uint32_t width = textureBindInfo.textureDataSources[d].width;
			uint32_t rowOffset = 0;
			if (!textureBindInfo.textureDataSources[d].imagePixelDatas.empty()) {
				for (uint32_t i = 0; i < height; i++)
				{
					//填充每一行
					rowOffset = sublayout.offset + d * sublayout.depthPitch + i * sublayout.rowPitch;
					FillImage(image, rowOffset, width * texdelByteSize, (const char*)textureBindInfo.textureDataSources[d].imagePixelDatas.data() + i * width * texdelByteSize);
				}
			
			
			}



		}
	}
	else {//目前这里只处理不是array类型的纹理
		uint32_t layer = textureBindInfo.textureDataSources.size();


		for (uint32_t l = 0; l < layer; l++)
		{
			auto& sublayout = image.layerMipLayouts[l][0];
			uint32_t width = textureBindInfo.textureDataSources[l].width;
			uint32_t height = textureBindInfo.textureDataSources[l].height;
			uint32_t rowOffset = 0;
			if (!textureBindInfo.textureDataSources[l].imagePixelDatas.empty()) {
				for (uint32_t i = 0; i < height; i++)
				{
					//填充每一行
					rowOffset = sublayout.offset + i * sublayout.rowPitch;
					FillImage(image, rowOffset, width * texdelByteSize, (const char*)textureBindInfo.textureDataSources[l].imagePixelDatas.data() + i * width * texdelByteSize);
				}
			}

		}
	
	
	
	}




}


void ExampleBaseVK::FillImage(Image& image, VkDeviceSize offset, VkDeviceSize size, const char* data)
{
	char* dst = (char*)image.hostMapPointer + offset;
	std::memcpy(dst, data, size);
}

void ExampleBaseVK::DestroyImage(Image& image)
{
	VulkanAPI::DestroyImage(device, image.image);
	DestroyImageView(device, image.imageView);
	ReleaseMemory(device, image.memory);
	image.image = nullptr;
	image.imageView = nullptr;
	image.memory = nullptr;
}



Texture ExampleBaseVK::CreateTexture(TextureBindInfo& textureBindInfo)
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
			if (i != 0)
			{
				ASSERT((x == textureBindInfo.textureDataSources[i - 1].width) &&
					(y == textureBindInfo.textureDataSources[i - 1].height));
			}

		}

	}
	//����texture
	uint32_t numLayer = textureBindInfo.textureDataSources.size();
	uint32_t numMip = uint32_t(log2(std::max(x, y))) + 1;
	if (textureBindInfo.buildMipmap)
	{

		texture.image.numMip = numMip;


	}
	VkImageType imageType = VK_IMAGE_TYPE_2D;
	uint32_t depth = 1;
	if (textureBindInfo.viewType == VK_IMAGE_VIEW_TYPE_3D)
	{
		imageType = VK_IMAGE_TYPE_3D;
		depth = numLayer;
		numLayer = 1;
	}
	
	if (textureBindInfo.format != TextureBindInfo::defaultTextureFormat)
	{
		ASSERT(CheckLinearFormatFeatureSupport(physicalDevice, textureBindInfo.format, TextureBindInfo::textureFormatFeatures));


	}
	

	texture.image = CreateImage(imageType, textureBindInfo.viewType, textureBindInfo.format, x, y, depth, texture.image.numMip, numLayer, textureBindInfo.usage, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VkComponentMapping{}, VK_IMAGE_TILING_LINEAR);
	texture.sampler = CreateDefaultSampler(device, 1);
	
	
	//获取每个layer以及mipmap的内存布局
	for (uint32_t layer = 0; layer < texture.image.numLayer; layer++) {
		for (uint32_t mip = 0; mip < texture.image.numMip; mip++)
		{

			VkImageSubresource subresource;
			subresource.aspectMask = texture.image.aspect;
			subresource.mipLevel = mip;
			subresource.arrayLayer = layer;
			auto sublayout = GetImageSubresourceLayout(device, texture.image.image, subresource);
			texture.image.layerMipLayouts[layer][mip] = sublayout;
		}

	}



	//从data source中获取数据填充image
	FillImageFromDataSource(texture.image, textureBindInfo);


	if (textureBindInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT)
	{
		TransferWholeImageLayout(texture.image, VK_IMAGE_LAYOUT_GENERAL);
	}
	else {
		TransferWholeImageLayout(texture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}


	
	
	
	//生成mipmap
	if (textureBindInfo.buildMipmap)
	{
		GenerateMipmap(texture.image);
		//texture.image.WriteToJpg("ttt0.jpg",0,0);
		//texture.image.WriteToJpg("ttt1.jpg", 0, 1);
		//texture.image.WriteToJpg("ttt2.jpg", 0, 2);
		//texture.image.WriteToJpg("ttt3.jpg", 0, 3);

		//texture.image.WriteToJpg("ttt4.jpg", 0, 4);
	}



	return texture;

}


void ExampleBaseVK::LoadObj(const std::string& objFilePath, Geometry& geo)
{
	//inner data
	geo.geoPath = objFilePath;
	auto& objDataSource =  objDataSourceCache[objFilePath];
	std::string warn, err;
	if (!tinyobj::LoadObj(&objDataSource.vertexAttrib, &objDataSource.shapes, &objDataSource.materials, &warn, &err, objFilePath.c_str()))
	{
		LogFunc(0);
	}

	auto& vertexAttibute = objDataSource.vertexAttrib;
	auto& shapes = objDataSource.shapes;


	//填充geometry的顶点缓冲数据以及索引数据
	uint32_t numVertex = vertexAttibute.vertices.size() / 3;
	geo.numVertex = numVertex;
	if (numVertex == 0)
	{
		return;
	}
	

	auto FillVertexBuffer = [&](uint32_t vertexIndex, uint32_t component, float* srcAttibuteData, std::vector<float>& dstVertexDatas) {
		std::memcpy(dstVertexDatas.data() + vertexIndex * vertexAttributeInputFloatStride + component * 3, srcAttibuteData, sizeof(float) * 3);

		};

	if (geo.useIndexBuffers)
	{

		geo.vertexAttributesDatas.resize(numVertex * vertexAttributeInputFloatStride);
		Geometry& geometry = geo;

		//����vertex buffer

		for (uint32_t vertexId = 0; vertexId < numVertex; vertexId++)
		{
			//��䶥��λ������
			//FillBuffer(geometry.vertexBuffer, vertexId * vertexAttributeInputStride, 3 * sizeof(float), (const char*)(geo.vertexAttrib.vertices.data() + vertexId * 3));
			FillVertexBuffer(vertexId, VAT_Position_float32, vertexAttibute.vertices.data() + vertexId * 3, geo.vertexAttributesDatas);


		}

		//填充颜色
		if (!vertexAttibute.colors.empty())
		{
			for (uint32_t vertexId = 0; vertexId < numVertex; vertexId++)
			{
				//��䶥��λ������
				//FillBuffer(geometry.vertexBuffer, vertexId * vertexAttributeInputStride + sizeof(float) * 3 * 2, 3 * sizeof(float), (const char*)(geo.vertexAttrib.colors.data() + vertexId * 3));
				FillVertexBuffer(vertexId, VAT_Color_float32, vertexAttibute.colors.data() + vertexId * 3, geo.vertexAttributesDatas);
			}


		}

		geo.shapeIndices.resize(shapes.size());

		geometry.indexBuffers.resize(shapes.size());

		//手动计算法线
		std::vector<glm::vec3> vertexNormals(numVertex, glm::vec3(0));
		std::map<uint32_t, std::set<uint32_t>> vertexNormalIds;

		//纹理坐标数据待定

		for (uint32_t zoneId = 0; zoneId < geometry.indexBuffers.size(); zoneId++)
		{

			for (uint32_t cellId = 0; cellId < shapes[zoneId].mesh.num_face_vertices.size(); cellId++)
			{
				if (shapes[zoneId].mesh.num_face_vertices[cellId] != 3)
				{
					LogFunc(0);//������������ε�ģ�;�ֱ�ӱ���
				}
			}
			auto& indicesData = geo.shapeIndices[zoneId];
			indicesData.resize(shapes[zoneId].mesh.num_face_vertices.size() * 3);
			std::map<uint32_t, uint32_t> vertexIdToTexId;
			for (uint32_t i = 0; i < shapes[zoneId].mesh.indices.size(); i++)
			{
				indicesData[i] = shapes[zoneId].mesh.indices[i].vertex_index;//��Ŷ�������
				const auto& normalIndex = shapes[zoneId].mesh.indices[i].normal_index;
				//�����������
				//填充法线
				if (!vertexAttibute.normals.empty())
				{
					glm::vec3 curNormal = glm::vec3(vertexAttibute.normals[normalIndex * 3], vertexAttibute.normals[normalIndex * 3 + 1], vertexAttibute.normals[normalIndex * 3 + 2]);
					curNormal = glm::normalize(curNormal);
					if (!vertexNormalIds[indicesData[i]].contains(normalIndex))//如果这是一个新的法线则加入计算
					{
						vertexNormalIds[indicesData[i]].insert(normalIndex);
						vertexNormals[indicesData[i]] += curNormal;
					}
				}

				//const auto& texCoordIndex = geo.shapes[zoneId].mesh.indices[i].texcoord_index;
				//auto vertexId = indicesData[i];
				////填充纹理坐标，这里假定所有顶点的纹理坐标都相同，否则报错
				//if (!geo.vertexAttrib.texcoords.empty())
				//{
				//	glm::vec3 curTexCoord = glm::vec3(geo.vertexAttrib.texcoords[texCoordIndex * 2], geo.vertexAttrib.texcoords[texCoordIndex * 2 + 1],1);
				//	if (vertexIdToTexId[vertexId] != 0 && vertexIdToTexId[vertexId]!= texCoordIndex)
				//	{
				//		ASSERT(0);//报错
				//	}
				//	vertexIdToTexId[vertexId] = texCoordIndex;
				//
				//	FillVertexBuffer(vertexId, VAT_TextureCoordinates_float32, (float*)&curTexCoord, geo.vertexAttributesDatas);
				//}

			}


		}


		for (uint32_t v = 0; v < numVertex; v++)
		{
			glm::vec3 curV = vertexNormals[v];
			//归一化
			curV = glm::normalize(curV);
			//填充法向量
			//FillBuffer(geometry.vertexBuffer, v * vertexAttributeInputStride + 3 * sizeof(float), 3 * sizeof(float), (const char*)(&curV));
			FillVertexBuffer(v, VAT_Normal_float32, (float*)&curV, geo.vertexAttributesDatas);
		}
	}
	else {
		geo.shapeVertexAttributesBuffers.resize(shapes.size());
		geo.shapeVertexBuffers.resize(shapes.size());

		for (uint32_t zoneId = 0; zoneId < geo.shapeVertexBuffers.size(); zoneId++)
		{
			std::vector<float>& curVertexData = geo.shapeVertexAttributesBuffers[zoneId];
			for (uint32_t cellId = 0; cellId < shapes[zoneId].mesh.num_face_vertices.size(); cellId++)
			{
				if (shapes[zoneId].mesh.num_face_vertices[cellId] != 3)
				{
					LogFunc(0);//������������ε�ģ�;�ֱ�ӱ���
				}
			}
			//当前shape的所有片元的点数据
			curVertexData.resize(shapes[zoneId].mesh.indices.size() * vertexAttributeInputFloatStride);
			for (uint32_t i = 0; i < shapes[zoneId].mesh.indices.size(); i++)
			{
				const auto& vertexIndex = shapes[zoneId].mesh.indices[i].vertex_index;//��Ŷ�������
				const auto& normalIndex = shapes[zoneId].mesh.indices[i].normal_index;
				const auto& texCoordIndex = shapes[zoneId].mesh.indices[i].texcoord_index;
				//�����������
				//填充顶点位置
				if (!vertexAttibute.vertices.empty())
				{
					FillVertexBuffer(i, VAT_Position_float32, vertexAttibute.vertices.data() + vertexIndex * 3, geo.shapeVertexAttributesBuffers[zoneId]);
				}

				//填充颜色
				if (!vertexAttibute.colors.empty())
				{
					FillVertexBuffer(i, VAT_Color_float32, vertexAttibute.colors.data() + vertexIndex * 3, geo.shapeVertexAttributesBuffers[zoneId]);
				}

				//填充法线
				if (!vertexAttibute.normals.empty())
				{
					glm::vec3 curNormal = glm::vec3(vertexAttibute.normals[normalIndex * 3], vertexAttibute.normals[normalIndex * 3 + 1], vertexAttibute.normals[normalIndex * 3 + 2]);
					curNormal = glm::normalize(curNormal);
					FillVertexBuffer(i, VAT_Normal_float32, (float*)&curNormal, geo.shapeVertexAttributesBuffers[zoneId]);

				}
				//填充纹理坐标
				if (!vertexAttibute.texcoords.empty())
				{
					glm::vec3 curTexCoord = glm::vec3(vertexAttibute.texcoords[texCoordIndex * 2], vertexAttibute.texcoords[texCoordIndex * 2 + 1], 1);
					//curTexCoord.y = 1-curTexCoord.y;//翻转一下y

					FillVertexBuffer(i, VAT_TextureCoordinates_float32, (float*)&curTexCoord, geo.shapeVertexAttributesBuffers[zoneId]);
				}
			}

		}

	}


	geo.CalculateAABBInfos();
}


Buffer ExampleBaseVK::CreateVertexBuffer(const char* buf, VkDeviceSize size)
{
	return CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

Buffer ExampleBaseVK::CreateIndexBuffer(const char* buf, VkDeviceSize size)
{
	return CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

Buffer ExampleBaseVK::CreateShaderAccessBuffer(const char* buf, VkDeviceSize size)
{
	return CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buf, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

void ExampleBaseVK::BuildAccelerationStructure(Geometry& geo)
{
	std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometryKHRs;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>  accelerationStructureBuildRangeInfoKHRs{};


	std::vector<char> acceleratBuffer;
	VkDeviceSize bufferTotalSize = 0;
	VkDeviceSize indexDataOffset = 0;
	uint32_t numVertex = 0;
	if (geo.useIndexBuffers)
	{
		accelerationStructureBuildRangeInfoKHRs.resize(1);
		uint32_t numTriangles = 0;
		for (uint32_t i = 0; i < geo.shapeIndices.size(); i++)
		{
			numTriangles += geo.shapeIndices[i].size() / 3;
		}
		numVertex = geo.vertexAttributesDatas.size() / vertexAttributeInputFloatStride;

		VkDeviceSize vertexAttributeDataSize = geo.vertexAttributesDatas.size() * sizeof(float);
		bufferTotalSize = vertexAttributeDataSize;
		indexDataOffset = vertexAttributeDataSize;
		auto& accelerationStructureBuildRangeInfoKHR = accelerationStructureBuildRangeInfoKHRs[0];
		accelerationStructureBuildRangeInfoKHR.firstVertex = geo.shapeIndices[0][0];//为geometry的三角形图元的第一个顶点的索引
		accelerationStructureBuildRangeInfoKHR.primitiveCount = numTriangles;//定义对应加速结构中geometry的图元数量
		accelerationStructureBuildRangeInfoKHR.primitiveOffset = 0;//指定图元数据在内存中的起始字节偏移量
		accelerationStructureBuildRangeInfoKHR.transformOffset = 0;//指定变换矩阵数据在内存中的起始字节偏移量
		bufferTotalSize += numTriangles * 3 * sizeof(uint32_t);

	
	}
	else {
		for (uint32_t i = 0; i < geo.shapeVertexAttributesBuffers.size(); i++)
		{
			bufferTotalSize += geo.shapeVertexAttributesBuffers[i].size() * sizeof(float);
			numVertex += geo.shapeVertexAttributesBuffers[i].size() / vertexAttributeInputFloatStride;
		}

	}
	
	//查询scratch 数据的最小字节对齐
	VkPhysicalDeviceAccelerationStructurePropertiesKHR physicalDeviceAccelerationStructurePropertiesKHR{};
	physicalDeviceAccelerationStructurePropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	physicalDeviceAccelerationStructurePropertiesKHR.pNext = nullptr;
	auto physicalDeviceProperties = GetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceAccelerationStructurePropertiesKHR);








	accelerationStructureGeometryKHRs.resize(1);

	auto& geometryKHR = accelerationStructureGeometryKHRs[0];
	geometryKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;;
	geometryKHR.pNext = nullptr;
	geometryKHR.flags = 0;
	geometryKHR.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;//描述了该加速结构引用的类型
	VkAccelerationStructureGeometryDataKHR geometryDataKHR{};
	{
		VkAccelerationStructureGeometryTrianglesDataKHR trianglesKHR{};
		{
			//
			trianglesKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			trianglesKHR.pNext = VK_NULL_HANDLE;
			trianglesKHR.vertexData = VkDeviceOrHostAddressConstKHR{ .deviceAddress/*通过vkGetBufferDeviceAddressKHR返回*/ = 0 };
			trianglesKHR.vertexStride = vertexAttributeInputStride;//为每个顶点间的字节步长大小
			trianglesKHR.maxVertex = numVertex - 1;//为vertexData 中的顶点数量减去1
			trianglesKHR.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;//为顶点元素的数据VkFormat格式
			trianglesKHR.indexData = VkDeviceOrHostAddressConstKHR{ .deviceAddress/*通过vkGetBufferDeviceAddressKHR返回*/ = 0 };//为包含索引数据的device 或者host端的地址
			trianglesKHR.indexType = VK_INDEX_TYPE_UINT32;//为索引元素的数据类型 VkIndexType			
			trianglesKHR.transformData = VkDeviceOrHostAddressConstKHR{ .deviceAddress/*通过vkGetBufferDeviceAddressKHR返回*/ = 0};//为包含可选的VkTransformMatrixKHR 数据的device 或者host端的地址，描述从geometry的顶点所在的空间到加速结构定义所在的空间的变换
		}
		geometryDataKHR.triangles = trianglesKHR;// VkAccelerationStructureGeometryTrianglesDataKHR 值 
		geometryKHR.geometry = geometryDataKHR;
	}

	//查询构建加速结构的大小信息



	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR{};
	accelerationStructureBuildGeometryInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfoKHR.pNext = nullptr;
	accelerationStructureBuildGeometryInfoKHR.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	VkDeviceOrHostAddressKHR scratchData{};
	{
		scratchData.deviceAddress = 0;
		scratchData.hostAddress = 0;
	}
	accelerationStructureBuildGeometryInfoKHR.scratchData = scratchData;
	accelerationStructureBuildGeometryInfoKHR.srcAccelerationStructure = VK_NULL_HANDLE;
	accelerationStructureBuildGeometryInfoKHR.dstAccelerationStructure = VK_NULL_HANDLE;
	accelerationStructureBuildGeometryInfoKHR.geometryCount = 1;
	accelerationStructureBuildGeometryInfoKHR.pGeometries = &geometryKHR;
	accelerationStructureBuildGeometryInfoKHR.ppGeometries = VK_NULL_HANDLE;
	accelerationStructureBuildGeometryInfoKHR.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationStructureBuildGeometryInfoKHR.flags = 0;


	auto buildSize = GetAccelerationStructureBuildSizesKHR(device, accelerationStructureBuildGeometryInfoKHR);

	bufferTotalSize = std::max(bufferTotalSize, buildSize.accelerationStructureSize);

	//填充buffer
	acceleratBuffer.resize(bufferTotalSize);
	if (geo.useIndexBuffers)
	{

		//填充buffer	
		std::memcpy(acceleratBuffer.data(), geo.vertexAttributesDatas.data(), geo.vertexAttributesDatas.size() * sizeof(float));
		VkDeviceSize offset = 0;
		for (uint32_t i = 0; i < geo.shapeIndices.size(); i++)
		{
			std::memcpy(acceleratBuffer.data() + indexDataOffset + offset, geo.shapeIndices[i].data(), geo.shapeIndices[i].size() * sizeof(uint32_t));
			offset += geo.shapeIndices[i].size() * sizeof(uint32_t);
		}

	}
	else {
		//填充buffer
		for (uint32_t i = 0; i < geo.shapeVertexAttributesBuffers.size(); i++)
		{
			std::memcpy(acceleratBuffer.data(), geo.shapeVertexAttributesBuffers[i].data(), geo.shapeVertexAttributesBuffers[i].size() * sizeof(float));
		}
	}


	//创建加速结构的buffer
	geo.accelerationStructureKHRBuffer = CreateBuffer( VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT , acceleratBuffer.data(), bufferTotalSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VkDeviceAddress bufferAddress = GetBufferDeviceAddressKHR(device, geo.accelerationStructureKHRBuffer.buffer);


	geo.accelerationStructureKHR = CreateAccelerationStructureKHR(device, 0, geo.accelerationStructureKHRBuffer.buffer, 0, bufferTotalSize, VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR);


	//创建scratch buffer
	geo.accelerationStructureScratchBuffer = CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, nullptr, buildSize.buildScratchSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VkDeviceAddress scratchDeviceAddress = GetBufferDeviceAddressKHR(device, geo.accelerationStructureScratchBuffer.buffer);


	//设置加速结构构建顶点索引数据buffer地址信息

	if (geo.useIndexBuffers)
	{
		geometryKHR.geometry.triangles.indexData = VkDeviceOrHostAddressConstKHR{ .deviceAddress/*通过vkGetBufferDeviceAddressKHR返回*/ = bufferAddress + indexDataOffset };//为包含索引数据的device 或者host端的地址
		geometryKHR.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;//为索引元素的数据类型 VkIndexType
	}
	else {

		geometryKHR.geometry.triangles.indexData = VkDeviceOrHostAddressConstKHR{ .deviceAddress/*通过vkGetBufferDeviceAddressKHR返回*/ = 0 };//为包含索引数据的device 或者host端的地址
		geometryKHR.geometry.triangles.indexType = VK_INDEX_TYPE_MAX_ENUM;//为索引元素的数据类型 VkIndexType
	}

	geometryKHR.geometry.triangles.vertexData = VkDeviceOrHostAddressConstKHR{ .deviceAddress/*通过vkGetBufferDeviceAddressKHR返回*/ = bufferAddress };

	VkDeviceOrHostAddressKHR scratchAddress = VkDeviceOrHostAddressKHR{ .deviceAddress = scratchDeviceAddress };

	//构建加速结构
	CmdListWaitFinish(oneSubmitCommandList);
	CmdListRecordBegin(oneSubmitCommandList);
	CmdBuildBottomAccelerationStructureKHR(oneSubmitCommandList.commandBuffer, accelerationStructureGeometryKHRs, accelerationStructureBuildRangeInfoKHRs, geo.accelerationStructureKHR, scratchAddress);
	CmdListRecordEnd(oneSubmitCommandList);
	SubmitSynchronizationInfo info;
	CmdListSubmit(oneSubmitCommandList, info);
}

void ExampleBaseVK::FillBuffer(Buffer buffer, VkDeviceSize offset, VkDeviceSize size, const char* data)
{
	ASSERT(buffer.size >= offset + size);
	char* dst = (char*)buffer.hostMapPointer + offset;
	std::memcpy(dst, data, size);

}

void ExampleBaseVK::ClearTexture(const std::string& textureName, VkClearColorValue clearValue)
{
	ASSERT(textures.contains(textureName));
	auto& image = textures[textureName].image;
	std::vector<VkImageSubresourceRange> imageRanges;
	imageRanges.resize(1);
	auto& range = imageRanges[0];
	range.aspectMask = image.aspect;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = image.numLayer;
	range.levelCount = image.numMip;

	std::vector<VkClearColorValue> clearVs = { clearValue };

	//构建加速结构
	CmdListWaitFinish(oneSubmitCommandList);
	CmdListRecordBegin(oneSubmitCommandList);
	CmdClearColorImage(oneSubmitCommandList.commandBuffer, image.image, image.currentLayout, clearVs, imageRanges);
	CmdListRecordEnd(oneSubmitCommandList);
	SubmitSynchronizationInfo info;
	CmdListSubmit(oneSubmitCommandList, info);

}

void ExampleBaseVK::CmdListReset(CommandList& cmdList)
{
	CommandBufferReset(cmdList.commandBuffer);
}

void ExampleBaseVK::CmdListRecordBegin(CommandList& cmdList)
{
	BeginRecord(cmdList.commandBuffer, cmdList.commandBufferUsage);
}

void ExampleBaseVK::CmdListRecordEnd(CommandList& cmdList)
{
	EndRecord(cmdList.commandBuffer);
}



void ExampleBaseVK::CmdOpsImageMemoryBarrer(CommandList& cmdList, Image& image, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout dstImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, int layer, int mip)
{
	auto imageBarrier = barrier.ImageBarrier(image, srcAccess, dstAccess, dstImageLayout,layer,mip);
	CmdMemoryBarrier(cmdList.commandBuffer, srcStageMask, dstStageMask, 0, {}, {}, { imageBarrier });
	image.currentLayout = dstImageLayout;
}

void ExampleBaseVK::CmdOpsDispatch(CommandList& cmdList, uint32_t computePassIndex, std::array<uint32_t, 3> groupSize)
{
	auto& computePipelineInfos = computePipelinesInfos[computePassIndex];
	CmdBindPipeline(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineInfos.pipeline);
	//绑定描述符集
	for (const auto& setInfo : computePipelineInfos.descriptorSetInfos)
	{
		CmdBindDescriptorSet(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineInfos.pipelineLayout, setInfo.first, { setInfo.second.descriptorSet }, {});
	}
	CmdDispatch(cmdList.commandBuffer, groupSize[0], groupSize[1], groupSize[2]);


}

void ExampleBaseVK::CmdOpsTraceRays(CommandList& cmdList, uint32_t rayTracingPipelineIndex,std::array<uint32_t, 3> groupSize)
{
	auto& rayTracingPipelineInfos = rayTracingPipelinesInfos[rayTracingPipelineIndex];
	CmdBindPipeline(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineInfos.pipeline);
	//绑定描述符集
	for (const auto& setInfo : rayTracingPipelineInfos.descriptorSetInfos)
	{
		CmdBindDescriptorSet(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineInfos.pipelineLayout, setInfo.first, { setInfo.second.descriptorSet }, {});
	}

	VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable = nullptr;
	VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable = nullptr;
	VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable = nullptr;
	VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable = nullptr;

	if (rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_RAYGEN_BIT_KHR].size != 0)
	{
		pRaygenShaderBindingTable = &rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_RAYGEN_BIT_KHR];
	}

	if (rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_MISS_BIT_KHR].size != 0)
	{
		pMissShaderBindingTable = &rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_MISS_BIT_KHR];
	}
	if (rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_ANY_HIT_BIT_KHR].size != 0)
	{
		pHitShaderBindingTable = &rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_ANY_HIT_BIT_KHR];
	}
	if (rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR].size != 0)
	{
		ASSERT(pHitShaderBindingTable == nullptr);
		pHitShaderBindingTable = &rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR];
	}
	if (rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_CALLABLE_BIT_KHR].size != 0)
	{
		pCallableShaderBindingTable = &rayTracingPipelinesInfos[rayTracingPipelineIndex].rayTracingShaderBindingRangeInfos[VK_SHADER_STAGE_CALLABLE_BIT_KHR];
	}
	CmdTraceRaysKHR(cmdList.commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, groupSize[0], groupSize[1], groupSize[2]);
}



void ExampleBaseVK::CmdOpsCopyWholeImageToImage(CommandList& cmdList,Image& srcImage, Image& dstImage)
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

void ExampleBaseVK::CmdOpsCopyImageToImage(CommandList& cmdList, Image& srcImage, uint32_t srcLayer, uint32_t srcMip, Image& dstImage, uint32_t dstLayer, uint32_t dstMip)
{
	VkImageCopy copyRegion{};
	copyRegion.srcOffset = VkOffset3D{ 0,0,0 };
	copyRegion.dstOffset = VkOffset3D{ 0,0,0 };

	//校验两个image的指定layer和mip的大小是否匹配
	auto srcMipExtent = srcImage.GetMipLevelExtent(srcMip);
	auto dstMipExtent = dstImage.GetMipLevelExtent(dstMip);
	if (srcMipExtent.width != dstMipExtent.width || srcMipExtent.height != dstMipExtent.height)
	{
		Log("copy : two image mip are not compatible ! ", 0);

	}

	copyRegion.extent = srcMipExtent;
	copyRegion.srcSubresource = VkImageSubresourceLayers{ .aspectMask = srcImage.aspect,
														  .mipLevel = srcMip,
														  .baseArrayLayer = srcLayer,
														  .layerCount = 1 };
	copyRegion.dstSubresource = VkImageSubresourceLayers{ .aspectMask = dstImage.aspect,
														  .mipLevel = dstMip,
														  .baseArrayLayer = dstLayer,
														  .layerCount = 1 };
	std::vector<VkImageCopy> copyRegions;
	copyRegions.push_back(copyRegion);
	CmdCopyImageToImage(cmdList.commandBuffer, srcImage.image, srcImage.currentLayout, dstImage.image, dstImage.currentLayout, copyRegions);

}

void ExampleBaseVK::CmdOpsBlitWholeImageToImage(CommandList& cmdList, Image& srcImage, Image& dstImage)
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

void ExampleBaseVK::CmdOpsClearWholeColorImage(CommandList& cmdList, Image& image, VkClearColorValue clearValue)
{
	std::vector<VkImageSubresourceRange> clearRanges;
	VkImageSubresourceRange wholeRange;
	wholeRange.aspectMask = image.aspect;
	wholeRange.baseArrayLayer = 0;
	wholeRange.layerCount = image.numLayer;
	wholeRange.baseMipLevel = 0;
	wholeRange.levelCount = image.numMip;



	clearRanges.push_back(wholeRange);
	std::vector<VkClearColorValue> clearColorValues;
	clearColorValues.push_back(clearValue);
	CmdClearColorImage(cmdList.commandBuffer, image.image, image.currentLayout, clearColorValues, clearRanges);


}

void ExampleBaseVK::CmdOpsDrawGeom(CommandList& cmdList, uint32_t renderPassIndex)
{
	//
	//if (renderTargets.colorAttachment.attachmentImage.currentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || renderTargets.depthAttachment.attachmentImage.currentLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	//{
	//	ASSERT(0);
	//}
	
	auto& renderPassInfo = renderPassInfos[renderPassIndex];
	auto& subpassDrawGeoInfos = renderPassInfo.subpassDrawGeoInfos;
	auto& subpassDrawGeoShapeInfos = renderPassInfo.subpassDrawGeoShapeInfos;
	auto& isMeshSubpass = renderPassInfo.isMeshSubpass;
	auto& subpassDrawMeshGroupInfos = renderPassInfo.subpassDrawMeshGroupInfos;
	auto& renderPass = renderPassInfo.renderPass;
	auto& frameBuffer = renderPassInfo.frameBuffer;
	auto& graphcisPipelineInfos = renderPassInfo.graphcisPipelineInfos;
	auto& renderTargets = renderPassInfo.renderTargets;

	ASSERT(subpassDrawGeoInfos.size() != 0 || subpassDrawMeshGroupInfos.size()!=0)
	uint32_t width = renderTargets.width;
	uint32_t height = renderTargets.height;
	std::vector<VkClearValue> attachmentClearValues;
	for (uint32_t i = 0; i < renderPassInfo.renderTargets.colorAttachments.size(); i++)
	{
		attachmentClearValues.push_back(renderTargets.colorAttachments[i].clearValue);
	}
	attachmentClearValues.push_back(renderTargets.depthAttachment.clearValue);


	CmdBeginRenderPass(cmdList.commandBuffer, renderPass, frameBuffer, VkRect2D{ .offset = VkOffset2D{.x = 0 ,.y = 0},.extent = VkExtent2D{.width = width,.height = height} }, attachmentClearValues, VK_SUBPASS_CONTENTS_INLINE);
	//每个subpass 会对应一个pipeline
	for (uint32_t curSubpassIndex = 0; curSubpassIndex < graphcisPipelineInfos.size(); curSubpassIndex++)
	{			


		//bind pipelines
		CmdBindPipeline(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphcisPipelineInfos[curSubpassIndex].pipeline);
		//绑定描述符集
		for (const auto& setInfo : graphcisPipelineInfos[curSubpassIndex].descriptorSetInfos)
		{
			CmdBindDescriptorSet(cmdList.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphcisPipelineInfos[curSubpassIndex].pipelineLayout, setInfo.first, { setInfo.second.descriptorSet }, {});
		}
		
		if (isMeshSubpass[curSubpassIndex])
		{
			auto& drawMeshGroupInfo = subpassDrawMeshGroupInfos[curSubpassIndex];
			CmdDrawMeshTasksEXT(cmdList.commandBuffer, drawMeshGroupInfo[0], drawMeshGroupInfo[1], drawMeshGroupInfo[2]);

		}
		else {
			for (uint32_t i = 0; i < subpassDrawGeoInfos[curSubpassIndex].size(); i++)
			{
				auto geoIndex = subpassDrawGeoInfos[curSubpassIndex][i];
				const auto& geom = geoms[geoIndex];
				
				
				if (geom.useIndexBuffers)
				{
					CmdBindVertexBuffers(cmdList.commandBuffer, 0, { geom.vertexBuffer.buffer }, { 0 });
					if (subpassDrawGeoShapeInfos[curSubpassIndex][geoIndex].empty())
					{
						for (uint32_t i = 0; i < geom.indexBuffers.size(); i++)
						{
							uint32_t numIndex = geom.shapeIndices[i].size();
							CmdBindIndexBuffer(cmdList.commandBuffer, geom.indexBuffers[i].buffer, 0, VK_INDEX_TYPE_UINT32);
							CmdDrawIndex(cmdList.commandBuffer, numIndex, 1, 0, 0, 0);
						}
					}
					else {
						for (uint32_t i = 0; i < subpassDrawGeoShapeInfos[curSubpassIndex][geoIndex].size(); i++)
						{
							uint32_t numIndex = geom.shapeIndices[i].size();
							auto shadpIndex = subpassDrawGeoShapeInfos[curSubpassIndex][geoIndex][i];
							CmdBindIndexBuffer(cmdList.commandBuffer, geom.indexBuffers[shadpIndex].buffer, 0, VK_INDEX_TYPE_UINT32);
							CmdDrawIndex(cmdList.commandBuffer, numIndex, 1, 0, 0, 0);
						}
					}

				}
				else {

					if (subpassDrawGeoShapeInfos[curSubpassIndex][geoIndex].empty())
					{

						for (uint32_t i = 0; i < geom.shapeVertexBuffers.size(); i++)
						{
							uint32_t numVertex = geom.shapeVertexAttributesBuffers[i].size() / vertexAttributeInputFloatStride;
							CmdBindVertexBuffers(cmdList.commandBuffer, 0, { geom.shapeVertexBuffers[i].buffer }, { 0 });
							CmdDrawVertex(cmdList.commandBuffer, numVertex, 1, 0, 0);
						}
	
					}
					else {
						for (uint32_t i = 0; i < subpassDrawGeoShapeInfos[curSubpassIndex][geoIndex].size(); i++)
						{
							uint32_t numVertex = geom.shapeVertexAttributesBuffers[i].size() / vertexAttributeInputFloatStride;
							auto shadpIndex = subpassDrawGeoShapeInfos[curSubpassIndex][geoIndex][i];
							CmdBindVertexBuffers(cmdList.commandBuffer, 0, { geom.shapeVertexBuffers[shadpIndex].buffer }, { 0 });
							CmdDrawVertex(cmdList.commandBuffer, numVertex, 1, 0, 0);
						}
					
					}

				}
				
				
				
				
				

			}
		
		
		
		}
		if (renderPassInfo.truncateNextSubpassDraw && curSubpassIndex == renderPassInfo.truncatedNextSubpassIndex - 1 && renderPassInfo.truncatedNextSubpassIndex !=0)
		{
			for (uint32_t i = curSubpassIndex; i < graphcisPipelineInfos.size() - 1; i++)
			{
				CmdNextSubpass(cmdList.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			}
			break;
		}

		if (curSubpassIndex != graphcisPipelineInfos.size() - 1)
		{
			CmdNextSubpass(cmdList.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	CmdEndRenderPass(cmdList.commandBuffer);
	


}

//如果一次提交执行的时间较长会导致vulkan报错，这个可能和显卡的关于执行长度管理的驱动具体实现相关
void ExampleBaseVK::CmdListSubmit(CommandList& cmdList, SubmitSynchronizationInfo& info)
{
	SubmitCommands(graphicQueue, info.waitSemaphores, info.waitStages, { cmdList.commandBuffer }, info.sigSemaphores, cmdList.commandFinishFence);
}

void ExampleBaseVK::CmdListWaitFinish(CommandList& cmdList)
{
	WaitAllFence({ cmdList.commandFinishFence });
	ResetAllFence({ cmdList.commandFinishFence });
}

VkResult ExampleBaseVK::Present(uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores)
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





void ExampleBaseVK::DestroyBuffer(Buffer& buffer)
{
	VulkanAPI::DestroyBuffer(device, buffer.buffer);
	ReleaseMemory(device, buffer.memory);
	buffer.buffer = nullptr;
	buffer.memory = nullptr;
}


void ExampleBaseVK::TransferWholeImageLayout(Image& image, VkImageLayout dstImageLayout)
{
	CmdListWaitFinish(oneSubmitCommandList);
	CmdListRecordBegin(oneSubmitCommandList);
	CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_NONE, VK_ACCESS_NONE, dstImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	CmdListRecordEnd(oneSubmitCommandList);
	
	SubmitSynchronizationInfo info;
	CmdListSubmit(oneSubmitCommandList, info);


}

void ExampleBaseVK::GenerateMipmap(Image& image)
{
	auto oldlayout = image.currentLayout;
	if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		ASSERT(0);//如果旧的layout未知这里就先直接报错
	}
	CmdListWaitFinish(oneSubmitCommandList);
	CmdListRecordBegin(oneSubmitCommandList);
	
	VkImageBlit blitRegion;
	uint32_t w = image.extent.width, h = image.extent.height;//layer 0的宽高
	for (uint32_t i = 1; i < image.numMip; i++)
	{
		CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,-1, i-1);
		CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,-1,i);
		
		auto srcMipExtent = image.GetMipLevelExtent(i - 1);
		auto dstMipExtent = image.GetMipLevelExtent(i);

		blitRegion.srcOffsets[0] = VkOffset3D{0,0,0};
		blitRegion.srcOffsets[1].x = srcMipExtent.width;
		blitRegion.srcOffsets[1].y = srcMipExtent.height;
		blitRegion.srcOffsets[1].z = srcMipExtent.depth;

		blitRegion.dstOffsets[0] = VkOffset3D{ 0,0,0 };
		blitRegion.dstOffsets[1].x = dstMipExtent.width;
		blitRegion.dstOffsets[1].y = dstMipExtent.height;
		blitRegion.dstOffsets[1].z = dstMipExtent.depth;

		blitRegion.srcSubresource = VkImageSubresourceLayers{
				.aspectMask = image.aspect,
				.mipLevel = i-1,
				.baseArrayLayer =0,
				.layerCount = image.numLayer
			
		};

		blitRegion.dstSubresource = VkImageSubresourceLayers{
		.aspectMask = image.aspect,
		.mipLevel = i,
		.baseArrayLayer = 0,
		.layerCount = image.numLayer

		};
		CmdBlitImageToImage(oneSubmitCommandList.commandBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { blitRegion });

		CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_NONE, oldlayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, -1, i - 1);
		CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_TRANSFER_WRITE_BIT , VK_ACCESS_NONE, oldlayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, -1, i);


	}
	CmdOpsImageMemoryBarrer(oneSubmitCommandList, image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE, oldlayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, -1, image.numMip-1);
	
	CmdListRecordEnd(oneSubmitCommandList);

	SubmitSynchronizationInfo info;
	CmdListSubmit(oneSubmitCommandList, info);


}

void ExampleBaseVK::InitGraphicPipelines(RenderPassInfo& renderPassInfo)
{
	auto& graphcisPipelineInfos = renderPassInfo.graphcisPipelineInfos;
	auto& subpassInfo = renderPassInfo.subpassInfo;
	auto& renderPass = renderPassInfo.renderPass;

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

						//如果是input attachment 则将附件对应信息存到绑定信息
						if (binding.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
						{
							graphcisPipelineInfos[pipeID].setBindingAttachmentIds[setId][binding.binding] = setAndBindings.second[bindingId].inputAttachmentIndex;

						}
						
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
		
		VkPipelineVertexInputStateCreateInfo* vertexInputState = &pipelineStates.vertexInputState;
		VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState = &pipelineStates.inputAssemblyState;
		if (enableMeshShaderEXT)
		{
			vertexInputState = nullptr;
			inputAssemblyState = nullptr;
		}

		auto pipeline = CreateGraphicsPipeline(device, 0, pipelineStates.shaderStages, vertexInputState, inputAssemblyState, &pipelineStates.tessellationState, &pipelineStates.viewportState,
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


void ExampleBaseVK::InitRecources()
{
	for (uint32_t i = 0; i < geoms.size(); i++)
	{
		InitGeometryResources(geoms[i]);
	}

	InitTextureResources();

	InitUniformBufferResources();


}

void ExampleBaseVK::InitQueryPool()
{
	queryPool = CreateQueryPool(device, 0, VK_QUERY_TYPE_PIPELINE_STATISTICS, 1, VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT);


}

void ExampleBaseVK::InitCommandList()
{
	graphicCommandList.commandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	graphicCommandList.commandBufferUsage = 0;
	graphicCommandList.commandFinishFence = CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT);
	oneSubmitCommandList.commandBuffer = AllocateCommandBuffer(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	oneSubmitCommandList.commandBufferUsage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	oneSubmitCommandList.commandFinishFence = CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT);

}

void ExampleBaseVK::Clear()
{
	//清除所有vulkan资源
	DeviceWaitIdle(device);
	ClearRayTracingPipeline();
	ClearComputePipeline();
	ClearRenderPasses();
	ClearRecources();
	ClearSyncObject();
	ClearQueryPool();

	ClearContex();

}

void ExampleBaseVK::ClearContex()
{				
	DesctroyCommandPool(device, commandPool);

	DestroySwapchain(device, swapchain);
	DestroyDevice(device);
	DestroySurface(instance, surface);
	DestroyGLFWWin32Window(window);
	DestroyDebugInfoMessager(instance,debugUtilMessager);
	DestroyInstance(instance);
}				  
				  
void ExampleBaseVK::ClearAttanchment(RenderPassInfo& renderPassInfo)
{				 
	auto& renderTargets = renderPassInfo.renderTargets;

	for (auto& colorAttachment : renderPassInfo.renderTargets.colorAttachments)
	{
		DestroyImage(colorAttachment.attachmentImage);
	}

	DestroyImage(renderTargets.depthAttachment.attachmentImage);
}				  
				  
void ExampleBaseVK::ClearRenderPass(RenderPassInfo& renderPassInfo)
{		
	auto& renderPass = renderPassInfo.renderPass;
	DestroyRenderPass(device, renderPass);

}

void ExampleBaseVK::ClearRenderPasses()
{
	for (uint32_t i = 0; i < renderPassInfos.size(); i++)
	{
		ClearGraphicPipelines(renderPassInfos[i]);
		ClearRenderPass(renderPassInfos[i]);
		ClearFrameBuffer(renderPassInfos[i]);
		ClearAttanchment(renderPassInfos[i]);

	}



}
				  
void ExampleBaseVK::ClearFrameBuffer(RenderPassInfo& renderPassInfo)
{				 
	auto& frameBuffer = renderPassInfo.frameBuffer;
	DestroyFrameBuffer(device, frameBuffer);
}				  
				  
void ExampleBaseVK::ClearGraphicPipelines(RenderPassInfo& renderPassInfo)
{				 
	auto& graphcisPipelineInfos = renderPassInfo.graphcisPipelineInfos;
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
				  
void ExampleBaseVK::ClearSyncObject()
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
				  
void ExampleBaseVK::ClearRecources()
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
		for (uint32_t zoneId = 0; zoneId < geoms[i].shapeVertexBuffers.size(); zoneId++)
		{
			DestroyBuffer(geoms[i].shapeVertexBuffers[zoneId]);
		}
		if (rayTracingPipelinsDesc.valid)
		{
			DestroyAccelerationStructureKHR(device, geoms[i].accelerationStructureKHR);
			DestroyBuffer(geoms[i].accelerationStructureKHRBuffer);
			DestroyBuffer(geoms[i].accelerationStructureScratchBuffer);
		}

	}

	//清除Uniform buffer
	for (auto& buffer : buffers)
	{
		DestroyBuffer(buffer.second);
	}


}				  
				  
void ExampleBaseVK::ClearQueryPool()
{
	DestroyQueryPool(device, queryPool);	
}

void ExampleBaseVK::ClearComputePipeline()
{
	if (computeDesc.valid)
	{
		for (uint32_t i = 0; i < computePipelinesInfos.size(); i++)
		{
			auto& computePipelineInfos = computePipelinesInfos[i];
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

}

void ExampleBaseVK::ClearRayTracingPipeline()
{
	if (rayTracingPipelinsDesc.valid)
	{
		for (uint32_t i = 0; i < rayTracingPipelinesInfos.size(); i++)
		{
			auto& rayTracingPipelineInfos = rayTracingPipelinesInfos[i];
			DestroyPipeline(device, rayTracingPipelineInfos.pipeline);
			DestroyPipelineLayout(device, rayTracingPipelineInfos.pipelineLayout);
			for (uint32_t setId = 0; setId < rayTracingPipelineInfos.descriptorSetInfos.size(); setId++)
			{
				//ReleaseDescriptorSets(device, graphcisPipelineInfos[i].descriptorPool, { graphcisPipelineInfos[i].descriptorSetInfos[setId].descriptorSet });
				DestroyDesctriptorSetLayout(device, rayTracingPipelineInfos.descriptorSetInfos[setId].setLayout);
			}
			DestroyDescriptorPool(device, rayTracingPipelineInfos.descriptorPool);
			DestroyBuffer(rayTracingPipelineInfos.stbBuffer);
		}



	}

}






void ExampleBaseVK::InitGeometryResources(Geometry& geo)
{
	if (geo.geoPath == "")
	{
		LogFunc(0);
	}


	if (geo.useIndexBuffers)
	{

		geo.indexBuffers.resize(geo.shapeIndices.size());

		for (uint32_t zoneId = 0; zoneId < geo.shapeIndices.size(); zoneId++)
		{

			auto& indicesData = geo.shapeIndices[zoneId];
			geo.indexBuffers[zoneId] = CreateIndexBuffer((const char*)indicesData.data(), indicesData.size() * sizeof(uint32_t));

		}
		if (geo.vertexAttributesDatas.empty())
		{
			return;
		}
		geo.vertexBuffer = CreateVertexBuffer((const char*)geo.vertexAttributesDatas.data(), geo.vertexAttributesDatas.size() * sizeof(float));




	}
	else {
		geo.shapeVertexBuffers.resize(geo.shapeVertexAttributesBuffers.size());
		
		for (uint32_t zoneId = 0; zoneId < geo.shapeVertexBuffers.size(); zoneId++)
		{
			std::vector<float>& curVertexData = geo.shapeVertexAttributesBuffers[zoneId];
			geo.shapeVertexBuffers[zoneId] = CreateVertexBuffer((const char*)curVertexData.data(), curVertexData.size() * sizeof(float));
			

		}
	}



	if (rayTracingPipelinsDesc.valid)
	{
		BuildAccelerationStructure(geo);
	}

}

void ExampleBaseVK::InitTextureResources()
{
	//根据texture infos创建texture并更新
	for (auto& textureInfo : textureBindInfos)
	{
		Texture texture;
		texture = CreateTexture(textureInfo.second);
		textures[textureInfo.first] = texture;
		
	}

}

void ExampleBaseVK::InitUniformBufferResources()
{
	for (const auto& bufferBindInfo : bufferBindInfos)
	{
		buffers[bufferBindInfo.first] = CreateShaderAccessBuffer(nullptr, bufferBindInfo.second.size);
		//BindUniformBuffer(uniformBuffers[unifomBufferInfo.first], unifomBufferInfo.second.pipeId, unifomBufferInfo.second.setId, unifomBufferInfo.second.binding, unifomBufferInfo.second.elementId);
	}

}

void ExampleBaseVK::InitCompute()
{
	if (computeDesc.valid)
	{
		computePipelinesInfos.resize(computeDesc.computeShaderPaths.size());
		for (uint32_t i = 0; i < computeDesc.computeShaderPaths.size(); i++)
		{
			//解析shader中的descriptor信息
			TransferGLSLFileToSPIRVFileAndRead(computeDesc.computeShaderPaths[i], computePipelinesInfos[i].computeShaderResourceInfo.spirvCode);
			ParseSPIRVShaderResourceInfo(computePipelinesInfos[i].computeShaderResourceInfo.spirvCode, computePipelinesInfos[i].computeShaderResourceInfo);
			InitComputePipeline(computePipelinesInfos[i]);

		}


	





	}
}

void ExampleBaseVK::InitComputePipeline(ComputePipelineInfos& computePipelineInfos)
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
	DestroyShaderModule(device, shaderModule);
}

void ExampleBaseVK::InitRayTracing()
{

	if (rayTracingPipelinsDesc.valid)
	{
		VkShaderStageFlagBits curShaderStage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		std::string curShaderPath = "";
		rayTracingPipelinesInfos.resize(rayTracingPipelinsDesc.raytracingPipelineShaderPaths.size());
		for (uint32_t i = 0; i < rayTracingPipelinsDesc.raytracingPipelineShaderPaths.size(); i++)
		{
			//解析shader中的descriptor信息
			auto& rayTracingPipelineInfos = rayTracingPipelinesInfos[i];
					//ray generate
			curShaderStage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			curShaderPath = rayTracingPipelinsDesc.raytracingPipelineShaderPaths[i].rayGenerateShaderPath;
			rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].shaderFilePath = curShaderPath;
			TransferGLSLFileToSPIRVFileAndRead(curShaderPath, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode);
			ParseSPIRVShaderResourceInfo(rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage]);


			curShaderStage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			curShaderPath = rayTracingPipelinsDesc.raytracingPipelineShaderPaths[i].closeHitShaderPath;
			rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].shaderFilePath = curShaderPath;
			TransferGLSLFileToSPIRVFileAndRead(curShaderPath, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode);
			ParseSPIRVShaderResourceInfo(rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage]);


			curShaderStage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			curShaderPath = rayTracingPipelinsDesc.raytracingPipelineShaderPaths[i].anyHitShaderPath;
			rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].shaderFilePath = curShaderPath;
			TransferGLSLFileToSPIRVFileAndRead(curShaderPath, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode);
			ParseSPIRVShaderResourceInfo(rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage]);

			curShaderStage = VK_SHADER_STAGE_MISS_BIT_KHR;
			curShaderPath = rayTracingPipelinsDesc.raytracingPipelineShaderPaths[i].missShaderPath;
			rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].shaderFilePath = curShaderPath;
			TransferGLSLFileToSPIRVFileAndRead(curShaderPath, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode);
			ParseSPIRVShaderResourceInfo(rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage]);

			curShaderStage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
			curShaderPath = rayTracingPipelinsDesc.raytracingPipelineShaderPaths[i].intersectionShaderPath;
			rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].shaderFilePath = curShaderPath;
			TransferGLSLFileToSPIRVFileAndRead(curShaderPath, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode);
			ParseSPIRVShaderResourceInfo(rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage]);
		
			curShaderStage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			curShaderPath = rayTracingPipelinsDesc.raytracingPipelineShaderPaths[i].callableShaderPath;
			rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].shaderFilePath = curShaderPath;
			TransferGLSLFileToSPIRVFileAndRead(curShaderPath, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode);
			ParseSPIRVShaderResourceInfo(rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage].spirvCode, rayTracingPipelineInfos.rayTracingshaderResourceInfos[curShaderStage]);


			InitRayTracingPipeline(rayTracingPipelineInfos);
		}

	}

}

void ExampleBaseVK::InitRayTracingPipeline(RayTracingPipelineInfos& rayTracingPipelineInfos)
{
	//��ʼ��shader stage


	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSetBindings;
	VkDescriptorSetLayoutBinding binding;
	std::map<VkDescriptorType, uint32_t> needNumDescriptor;


	for (const auto& kv : rayTracingPipelineInfos.rayTracingshaderResourceInfos)
	{
		if (kv.second.spirvCode.size() != 0)
		{



			//����spirv code����shader module
			auto shaderModule = CreateShaderModule(device, 0, kv.second.spirvCode);
			rayTracingPipelineInfos.rayTracingShaderBindingRangeInfos[kv.first].size = 1;


			//��ʼ��shader state ��Ϣ
			VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{ };
			pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineShaderStageCreateInfo.pNext = nullptr;
			pipelineShaderStageCreateInfo.flags = 0;
			pipelineShaderStageCreateInfo.stage = kv.first;
			pipelineShaderStageCreateInfo.module = shaderModule;
			pipelineShaderStageCreateInfo.pName = kv.second.entryName.c_str();
			pipelineShaderStageCreateInfo.pSpecializationInfo = nullptr;
			rayTracingPipelineInfos.rayTracingShaderStages.push_back(pipelineShaderStageCreateInfo);

			//��ȡ�󶨺�set��Ϣ
			//auto& bindins = kv.second.descriptorSetBindings;

			for (auto& setAndBindings : kv.second.descriptorSetBindings)
			{
				auto setId = setAndBindings.first;
				for (auto bindingId = 0; bindingId < setAndBindings.second.size(); bindingId++)
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
	rayTracingPipelineInfos.descriptorPool = descriptorPool;


	//����descriptor set
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	for (const auto& setAndBindingInfos : descriptorSetBindings)
	{
		//����descriptor set layout
		auto descriptorSetLayout = CreateDescriptorSetLayout(device, 0, setAndBindingInfos.second);
		rayTracingPipelineInfos.descriptorSetInfos[setAndBindingInfos.first].setLayout = descriptorSetLayout;
		auto descriptorSet = AllocateDescriptorSet(device, rayTracingPipelineInfos.descriptorPool, { descriptorSetLayout });
		rayTracingPipelineInfos.descriptorSetInfos[setAndBindingInfos.first].descriptorSet = descriptorSet;
		descriptorSetLayouts.push_back(descriptorSetLayout);
		rayTracingPipelineInfos.descriptorSetInfos[setAndBindingInfos.first].bindings = descriptorSetBindings[setAndBindingInfos.first];
	}

	//����pipeline layout
	auto pipelineLayout = CreatePipelineLayout(device, 0, descriptorSetLayouts, {});
	rayTracingPipelineInfos.pipelineLayout = pipelineLayout;



	//��ʼ��pipeline states

	auto& pipelineStates = rayTracingPipelineInfos.rayTracingShaderStages;

	//为了方便，每个shader stage创建一个shader group
	auto pipeline = CreateRayTracingPipeline(device, pipelineStates, 10, pipelineLayout);
	rayTracingPipelineInfos.pipeline = pipeline;


	//delete shader module
	for (const auto& shaderStageInfo : rayTracingPipelineInfos.rayTracingShaderStages)
	{
		DestroyShaderModule(device, shaderStageInfo.module);
	}



	//初始化ray tracing 的shader binding table


	//获取shader group句柄长度

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	rayTracingPipelineProperties.pNext = nullptr;

	auto physicalDeviceProperties = GetPhysicalDeviceProperties2(physicalDevice, &rayTracingPipelineProperties);
	uint32_t shaderGroupStride = std::max(rayTracingPipelineProperties.shaderGroupBaseAlignment, rayTracingPipelineProperties.shaderGroupHandleSize);


	std::vector<char> tmp;
	tmp.resize(rayTracingPipelineProperties.shaderGroupHandleSize* pipelineStates.size());
	GetRayTracingShaderGroupHandlesKHR(device, pipeline, 0, pipelineStates.size(), tmp);

	std::vector<char> stbBuffer;
	stbBuffer.resize(pipelineStates.size()* shaderGroupStride);
	for (uint32_t i = 0; i < pipelineStates.size(); i++)
	{
		std::memcpy(stbBuffer.data() + i * shaderGroupStride, tmp.data() + i * rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleSize);

	}




	//创建shader binding table
	rayTracingPipelineInfos.stbBuffer = CreateBuffer(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, stbBuffer.data(), stbBuffer.size(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VkDeviceAddress stbBufferAddress = GetBufferDeviceAddressKHR(device, rayTracingPipelineInfos.stbBuffer.buffer);


	VkDeviceSize offset = 0;

	for (uint32_t i = 0; i < pipelineStates.size(); i++)
	{
		auto& shaderStage = pipelineStates[i].stage;
		rayTracingPipelineInfos.rayTracingShaderBindingRangeInfos[shaderStage].deviceAddress = stbBufferAddress + offset;
		rayTracingPipelineInfos.rayTracingShaderBindingRangeInfos[shaderStage].size = shaderGroupStride;
		rayTracingPipelineInfos.rayTracingShaderBindingRangeInfos[shaderStage].stride = shaderGroupStride;
		offset += shaderGroupStride;
		VkDeviceSize ext = (stbBufferAddress + offset) % rayTracingPipelineProperties.shaderGroupBaseAlignment;
	}


}



void ExampleBaseVK::ReadGLSLShaderFile(const std::string& shaderPath, std::vector<char>& shaderCode)
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

bool ExampleBaseVK::CompileGLSLToSPIRV(VkShaderStageFlagBits shaderStage, const std::vector<char>& srcCode, std::vector<uint32_t>& outSpirvCode)
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


void ExampleBaseVK::TransferGLSLFileToSPIRVFileAndRead(const std::string& srcGLSLFile, std::vector<uint32_t>& outSpirvCode)
{
	if (srcGLSLFile == "")
	{
		return;
	}
	std::string vulkanIncludeDir(VULKAN_INCLUDE_DIRS);
	uint32_t pos = vulkanIncludeDir.find_last_of("/");
	std::string vulkanInstallDir = vulkanIncludeDir.substr(0, pos);
	std::string glslcDir = vulkanInstallDir + "/Bin/glslc.exe";
	std::string generateCmd = glslcDir + " " + srcGLSLFile + " -o " + "tmp.spv  --target-env=vulkan1.4";
	int ret = system(generateCmd.c_str());
	if (ret != 0)
	{
		ASSERT(0);
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

void ExampleBaseVK::ParseSPIRVShaderInputAttribute(const std::vector<uint32_t>& spirvCode, std::vector<ShaderInputAttributeInfo>& dstCacheShaderInputAttributeInfo)
{
	if (spirvCode.empty())
	{
		return;
	}
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

void ExampleBaseVK::ParseSPIRVShaderResourceInfo(const std::vector<uint32_t>& spirvCode, ShaderResourceInfo& dstCacheShaderResource)
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

	//����store buffer��Ϣ
	for (uint32_t i = 0; i < resources.storage_buffers.size(); i++)
	{
		auto& storeBuffer = resources.storage_buffers[i];
		auto type = shaderCompiler.get_type(storeBuffer.base_type_id);
		bindingInfo.setId = shaderCompiler.get_decoration(storeBuffer.id, spv::Decoration::DecorationDescriptorSet);
		bindingInfo.binding = shaderCompiler.get_decoration(storeBuffer.id, spv::DecorationBinding);
		bindingInfo.name = storeBuffer.name;
		if (type.array.empty())
		{
			bindingInfo.numDescriptor = 1;
		}
		else {
			bindingInfo.numDescriptor = type.array[0];
		}
		//bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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

	//����input attachment��Ϣ
	for (uint32_t i = 0; i < resources.subpass_inputs.size(); i++)
	{
		auto& inputAttachment = resources.subpass_inputs[i];
		auto type = shaderCompiler.get_type(inputAttachment.base_type_id);
		bindingInfo.setId = shaderCompiler.get_decoration(inputAttachment.id, spv::Decoration::DecorationDescriptorSet);
		bindingInfo.binding = shaderCompiler.get_decoration(inputAttachment.id, spv::DecorationBinding);
		bindingInfo.inputAttachmentIndex = shaderCompiler.get_decoration(inputAttachment.id, spv::DecorationInputAttachmentIndex);
		bindingInfo.name = inputAttachment.name;
		if (type.array.empty())
		{
			bindingInfo.numDescriptor = 1;
		}
		else {
			bindingInfo.numDescriptor = type.array[0];
		}
		//bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		dstCacheShaderResource.descriptorSetBindings[bindingInfo.setId].push_back(bindingInfo);
		
	}


	//acceleration struct
	for (uint32_t i = 0; i < resources.acceleration_structures.size(); i++)
	{
		auto& acceleration = resources.acceleration_structures[i];
		auto type = shaderCompiler.get_type(acceleration.base_type_id);
		bindingInfo.setId = shaderCompiler.get_decoration(acceleration.id, spv::Decoration::DecorationDescriptorSet);
		bindingInfo.binding = shaderCompiler.get_decoration(acceleration.id, spv::DecorationBinding);
		bindingInfo.name = acceleration.name;
		if (type.array.empty())
		{
			bindingInfo.numDescriptor = 1;
		}
		else {
			bindingInfo.numDescriptor = type.array[0];
		}
		//bindingInfo.numDescriptor = shaderCompiler.get_declared_struct_size(type);
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		dstCacheShaderResource.descriptorSetBindings[bindingInfo.setId].push_back(bindingInfo);

	}



}



Buffer ExampleBaseVK::CreateBuffer(VkBufferUsageFlags usage, const char* buf, VkDeviceSize size, VkMemoryPropertyFlags memoryPropties)
{
	Buffer buffer;
	buffer.usage = usage;
	buffer.size = size;
	buffer.memoryPropties = memoryPropties;
	buffer.buffer = VulkanAPI::CreateBuffer(device, 0, size, usage, VK_SHARING_MODE_EXCLUSIVE, {queueFamilyIndex});
	//�����ڴ�
	auto memRequirements = GetBufferMemoryRequirements(device, buffer.buffer);
	auto memtypeIndex = GetMemoryTypeIndex(memRequirements.memoryTypeBits, memoryPropties);
	VkMemoryAllocateFlags memoryAllocationFlags = 0;
	if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		memoryAllocationFlags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	}

	buffer.memory = AllocateMemory(device, memRequirements.size, memtypeIndex, memoryAllocationFlags);
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




uint32_t ExampleBaseVK::GetNextPresentImageIndex(VkSemaphore sigValidSemaphore)
{
	uint32_t nextImageIndex = VulkanAPI::GetNextValidSwapchainImageIndex(device, swapchain, sigValidSemaphore, nullptr);
	return nextImageIndex;
}

void ExampleBaseVK::SortGeosFollowCloseDistance(glm::vec3 cameraPos)
{
	for (uint32_t passId = 0; passId < renderPassInfos.size(); passId++)
	{
		for (uint32_t subpassId = 0; subpassId < renderPassInfos[passId].subpassDrawGeoInfos.size(); subpassId++)
		{
			std::sort(renderPassInfos[passId].subpassDrawGeoInfos[subpassId].begin(), renderPassInfos[passId].subpassDrawGeoInfos[subpassId].end(), [&](uint32_t geo1, uint32_t geo2) {
				return geoms[geo1].CloserThanOther(geoms[geo2], cameraPos);
				});
		}
	}

}

VkDescriptorType ExampleBaseVK::GetDescriptorType(DescriptorSetInfo& descriptorSetInfo, uint32_t binding)
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

void ExampleBaseVK::BindTexture(const Texture& texture, VkDescriptorSet set, uint32_t binding, uint32_t elemenId, VkDescriptorType descriptorType)
{
	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = texture.sampler;
	descriptorImageInfo.imageLayout = texture.image.currentLayout;
	descriptorImageInfo.imageView = texture.image.imageView;
	
	UpdateDescriptorSetBindingResources(device, set, binding, elemenId, 1, descriptorType, { descriptorImageInfo }, {}, {});


}

void ExampleBaseVK::BindBuffer(const Buffer& uniformBuffer, VkDescriptorSet set, uint32_t binding, uint32_t elemenId, VkDescriptorType descriptorType)
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = uniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = uniformBuffer.size;
	UpdateDescriptorSetBindingResources(device, set, binding, elemenId, 1, descriptorType, {  }, { descriptorBufferInfo }, {});

}

void ExampleBaseVK::BindAccelerationStructure(uint32_t geometryIndex, VkDescriptorSet set, uint32_t binding, uint32_t elemenId, VkDescriptorType descriptorType)
{
	VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrite = {};
	accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureWrite.accelerationStructureCount = 1;
	accelerationStructureWrite.pAccelerationStructures = &geoms[geometryIndex].accelerationStructureKHR; // TLAS 的句柄
	UpdateDescriptorSetBindingResources(device, set, binding, elemenId, 1, descriptorType, {  }, { }, {},&accelerationStructureWrite);

}

void ExampleBaseVK::BindInputAttachment(RenderPassInfo& renderPassInfo)
{



	for (auto& pipelineInfo : renderPassInfo.graphcisPipelineInfos)
	{
		for (auto& setBindingAttachmentInfo : pipelineInfo.setBindingAttachmentIds)
		{
			auto& set = pipelineInfo.descriptorSetInfos[setBindingAttachmentInfo.first].descriptorSet;
			for (auto& bindingAttachmentId : setBindingAttachmentInfo.second)
			{
				auto& binding = bindingAttachmentId.first;
				auto& attachmentId = bindingAttachmentId.second;
				VkDescriptorImageInfo descriptorImageInfo{};
				descriptorImageInfo.sampler = nullptr;
				descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
				descriptorImageInfo.imageView = renderPassInfo.renderTargets.colorAttachments[attachmentId].attachmentImage.imageView;

				UpdateDescriptorSetBindingResources(device, set, binding, 0, 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, { descriptorImageInfo }, {}, {});

			}

		}



	}




}


void ExampleBaseVK::WaitIdle()
{
	WaitQueueIdle(graphicQueue);

}

void ExampleBaseVK::WaitAllFence(const std::vector<VkFence>& fences)
{
	auto res = WaitFence(device, fences, true);
	if (res != VK_SUCCESS)
	{
		auto status = GetFenceStatus(device, fences[0]);
		ASSERT(0);
	}
}

void ExampleBaseVK::ResetAllFence(const std::vector<VkFence>& fences)
{
	ResetFences(device, fences);
}

//void ExampleBaseVK::CreateFences(uint32_t numFences)
//{
//	fences.resize(numFences);
//	for (uint32_t i = 0; i < numFences; i++)
//	{
//		fences[i] = CreateFence(device, VK_FENCE_CREATE_SIGNALED_BIT);
//	}
//}

void ExampleBaseVK::CreateSemaphores(uint32_t numSemaphores)
{
	semaphores.resize(numSemaphores);
	for (uint32_t i = 0; i < numSemaphores; i++)
	{
		semaphores[i] = Create_Semaphore_(device, 0);
	}


}

void ExampleBaseVK::SetSupassDescription(VkSubpassDescription& subpassDesc, VkSubpassDescriptionFlags flags, VkPipelineBindPoint pipelineBindPoint, const std::vector<VkAttachmentReference>& inputAttachmentRefs, const std::vector<VkAttachmentReference>& colorAttachmentRefs, const VkAttachmentReference* resolveAttachments, const VkAttachmentReference* depthStencilAttachment, const std::vector<uint32_t>& preserveAttachments)
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

bool ExampleBaseVK::CheckLinearFormatFeatureSupport(VkPhysicalDevice curPhysicalDevive, VkFormat format, VkFormatFeatureFlags features)
{
	auto colorFormatProps = GetFormatPropetirs(curPhysicalDevive, format);
	if (!((colorFormatProps.linearTilingFeatures & features) == features))
	{
		Log("the linear props does not support specific features",0);
		std::cout << int(colorFormatProps.linearTilingFeatures & features) << std::endl;
		//PrintSupportedFormatFeatures(colorFormatProps.linearTilingFeatures);
		return false;
	}

	return true;
}

bool ExampleBaseVK::CheckOptimalFormatFeatureSupport(VkPhysicalDevice curPhysicalDevive, VkFormat format, VkFormatFeatureFlags features)
{
	auto colorFormatProps = GetFormatPropetirs(curPhysicalDevive, format);
	if (!((colorFormatProps.optimalTilingFeatures & features) == features))
	{
		Log("the optimal props does not support specific features", 0);
		//PrintSupportedFormatFeatures(colorFormatProps.linearTilingFeatures);
		return false;
	}

	return true;
}

void ExampleBaseVK::PrintSupportedFormatFeatures(VkFormatFeatureFlags features)
{
	std::cout << "current supported features:" << std::endl;
	for (auto feature = VkFormatFeatureFlagBitsToString.begin(); feature != VkFormatFeatureFlagBitsToString.end(); feature++)
	{
		if ((features & feature->first) == feature->first) {
			std::cout << feature->second << std::endl;		
		}


	}

}

void ExampleBaseVK::FindFormat(VkPhysicalDevice curPhysicalDevive, VkFormatFeatureFlags features)
{
	std::cout << "find format by features:" << std::endl;
	uint32_t i = 0;
	for (auto iter = VkFormatToInfo.begin(); iter != VkFormatToInfo.end(); iter++)
	{
		bool res = CheckOptimalFormatFeatureSupport(curPhysicalDevive, iter->first, features);
		if (res)
		{
			//ASSERT(0);
			std::cout << "this format is surpported for this feature : " << iter->second.name << std::endl;
		}

	}
}

bool ExampleBaseVK::CheckExtensionSupport(VkPhysicalDevice curPhysicalDevive, const std::vector<const char*> entensions)
{
	auto supportedExtensions = EnumerateDeviceExtensionProperties(curPhysicalDevive, nullptr);

	uint32_t numSurpportedExtension = 0;
	for (auto& wantExtensionName : entensions) {
		for (auto& extensionProp : supportedExtensions) {
			if (std::strcmp(extensionProp.extensionName, wantExtensionName) == 0) {
			
				numSurpportedExtension++;
			}

		}
	
	}
	if (numSurpportedExtension == entensions.size())
	{
		return true;
	}
	return false;
}

