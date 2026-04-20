#include "VulkanRHIApi.h"
#include <iostream>
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanResource.h"
#include "RHIUtils.h"
#include "VulkanSwapchain.h"
#include "VulkanPipelineState.h"
#include "VulkanPlatformSurport.h"
#include "VulkanCommandContex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemory.h"
#include "VulkanRHIUtils.h"
#include "VulkanShader.h"
#include "VulkanTransientResource.h"
#include "RHIPipelineStateCache.h"
#include "VulkanBarriers.h"
#include "RHITransition.h"
#include "VulkanSwapchain.h"
#include "VulkanQueue.h"
#include "RHICaptureHelper.h"

#define DynamicPtrCast(ptr, type) (std::dynamic_pointer_cast<type>(ptr))
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	// 如果严重程度大于警告，可以使用红色输出或断点
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		fprintf(stderr, "Validation Layer: %s\n", pCallbackData->pMessage);
	}

	return VK_FALSE; // 永远返回 FALSE，否则 API 会在报错处中断并返回错误码
}

using namespace  RHI;
namespace RHIVulkan{

// 析构函数实现
    VulkanRHIApi::~VulkanRHIApi()
{
    Shutdown();
}

// 初始化和销毁接口实现
bool VulkanRHIApi::Init()
{
	if (ValidFlag) {
		return true;
	}
	RHICaptureHelper::GetInstance();
	VKFunc::InitializeLoader();
	GShaderPlatform = ERHIShaderPlatform::Vulkan;
    // 创建Vulkan实例
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "WREngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "WREngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // 可以在这里添加扩展和验证层
	auto wantExtensions = GetWantedInstanceExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(wantExtensions.size());
	createInfo.ppEnabledExtensionNames = wantExtensions.data();
	// 这里可以添加更多的实例创建信息，如验证层等
	auto wantedLayers = GetWantedInstanceLayers();
	createInfo.enabledLayerCount = static_cast<uint32_t>(wantedLayers.size());
	createInfo.ppEnabledLayerNames = wantedLayers.data();


    // 创建Vulkan实例
    bool result = VKFunc::CreateInstance(&createInfo, &Instance);
    if (result != true)
    {
        // 处理错误
        return false;
    }
#ifdef DEBUG_INFO
	// 设置调试回调（如果需要）
	// 这里可以添加创建 Debug Messenger 的逻辑
	VkDebugUtilsMessengerCreateInfoEXT debugMessagerCreateInfo{};
	debugMessagerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	// 设置你关心的消息严重程度
	debugMessagerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	// 设置你关心的消息类型
	debugMessagerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	// 指定回调函数
	debugMessagerCreateInfo.pfnUserCallback = debugCallback;
	debugMessagerCreateInfo.pUserData = nullptr; // 可选

	if (!VKFunc::CreateDebugUtilsMessengerEXT(Instance, &debugMessagerCreateInfo,  &DebugMessenger) != VK_SUCCESS) {
		assert(0);
	}
#endif

	RHI::RHICaptureHelper::GetInstance().Init();
	// 初始化Vulkan设备和其他资源
	// 这里可以添加更多的初始化逻辑，如选择物理设备、创建逻辑设备等
	PhysicalDevice = PickPhysicalDevice();
	if (PhysicalDevice == VK_NULL_HANDLE)
	{
		return false;
	}
	VulkanDevice* device = new VulkanDevice(this, PhysicalDevice);

	
	wantExtensions = GetWantedDeviceExtensions();
	wantedLayers = GetWantedDeviceLayers();

	if (!device->Init(wantedLayers, wantExtensions)) {
		delete device;
		device = nullptr;
		return false;
	}
	Device = device;

	// 1. 确定头大小
	uint32_t HeaderSize = sizeof(RHITransition);

	// 2. 确定私有数据的大小和对齐要求
	// 注意：如果你的私有数据需要动态支持多个 Barrier，
	// 这里的 PrivateSize 可以预留一个“常用最大值”，或者只存固定头
	uint32_t PrivateSize = sizeof(VulkanPipelineBarrier);
	uint32_t Alignment = alignof(VulkanPipelineBarrier);

	// 3. 计算偏移量 (对齐 Header 之后的地址)
	G_RHITransition_PrivateDataOffset = (HeaderSize + Alignment - 1) & ~(Alignment - 1);

	// 4. 计算总分配大小
	G_RHITransition_TotalSize = G_RHITransition_PrivateDataOffset + PrivateSize;

	ValidFlag = true;
    return true;
    // 初始化其他Vulkan资源
    // 如物理设备、逻辑设备、队列等
}

void VulkanRHIApi::Shutdown()
{
	if (!ValidFlag) {
		return;
	}

	ValidFlag = false;
	RHI::RHIPipelineStateCache::ClearAll();

	delete Device;
	Device = nullptr;


    // 销毁Vulkan实例和其他资源
    if (Instance != VK_NULL_HANDLE)
    {
#ifdef DEBUG_INFO
		VKFunc::DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger);
#endif
        VKFunc::DestroyInstance(Instance);
        Instance = VK_NULL_HANDLE;
    }

    // 销毁其他Vulkan资源
}


// 资源创建接口实现
RHITextureSP VulkanRHIApi::CreateTexture(const RHITextureDesc& desc)
{
    // 创建Vulkan纹理资源
    // 这里需要实现Vulkan特定的纹理创建逻辑
    return DynamicPtrCast(std::make_shared<VulkanTexture>(Device, desc),RHITexture); // 暂时返回nullptr，实际实现需要返回有效的纹理对象
}

RHIBufferSP VulkanRHIApi::CreateBuffer(const RHIBufferDesc& desc)
{
    // 创建Vulkan缓冲区资源
    // 这里需要实现Vulkan特定的缓冲区创建逻辑
    return DynamicPtrCast(std::make_shared<VulkanBuffer>(Device, desc), RHIBuffer); // 暂时返回nullptr，实际实现需要返回有效的缓冲区对象
}




void VulkanRHIApi::UpdateTexture(RHICommandListBase& cmdList, RHITexture* texture, const void* data,const RHITextureRegion& region)
{
	if (!texture || !data)
		return;
	auto format = texture->GetDesc().Format;

	VkDeviceSize totalSize = region.width * region.height * region.depth * RHI::GFormatInfoMap.at(format).BytesPerPixel; // 假设格式是RGBA8


	// 1. 获取 staging buffer
	auto staging = Device->GetStagingManager()->Acquire(totalSize);
	void* mapped = staging->Map(0, totalSize);


	// 复制数据到 staging buffer
	memcpy(mapped, data, totalSize);

	auto VkFormat = TransformFormatFrom(format);
	auto imageFlag = GetImageAspectFlags(VkFormat);

	// 3. 记录 CopyBufferToImage 命令
	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = imageFlag;
	copyRegion.imageSubresource.mipLevel = region.mipLevel;
	copyRegion.imageSubresource.baseArrayLayer = region.arraySlice;
	copyRegion.imageSubresource.layerCount = region.numArraySlices;
	copyRegion.imageOffset = { static_cast<int32_t>(region.xOffset),
	static_cast<int32_t>(region.yOffset),
	static_cast<int32_t>(region.zOffset) };
	copyRegion.imageExtent = { region.width, region.height, region.depth };

	VulkanTexture* vulkanTexture = dynamic_cast<VulkanTexture*>(texture);

	cmdList.AddCommand<VulkanCommandUpdateTexture>(vulkanTexture, staging, copyRegion);
	

}
void VulkanRHIApi::UpdateBuffer(RHICommandListBase& cmdList, RHIBuffer* buffer, const void* data, const RHIBufferRegion& region)
{
	if (!buffer || !data)
		return;

	VkDeviceSize totalSize = region.size;

	// 1. 获取 staging buffer
	auto staging = Device->GetStagingManager()->Acquire(totalSize);
	void* mapped = staging->Map(0, totalSize);

	// 2. 拷贝 CPU 数据
	memcpy(mapped, data, totalSize);

	// 3. 构造 CopyBuffer 区域
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = region.offset;
	copyRegion.size = totalSize;

	VulkanBuffer* vulkanBuffer = dynamic_cast<VulkanBuffer*>(buffer);

	// 4. 记录 command
	cmdList.AddCommand<VulkanCommandUpdateBuffer>(vulkanBuffer, staging, copyRegion);
}

RHIShaderResourceViewSP VulkanRHIApi::CreateTextureShaderResourceView(
	RHITexture* Texture, const RHITexSRVCreateInfo& Desc) 
{
	return DynamicPtrCast(std::make_shared<VulkanShaderResourceView>(Device, Texture, Desc), RHIShaderResourceView);
}

RHIUnorderedAccessViewSP VulkanRHIApi::CreateTextureUnorderedAccessView(
	RHITexture* Texture, const RHITexUAVCreateInfo& Desc)
{
	return DynamicPtrCast(std::make_shared<VulkanUnorderedAccessView>(Device, Texture, Desc), RHIUnorderedAccessView);
}

RHIShaderResourceViewSP VulkanRHIApi::CreateBufferShaderResourceView(
	RHIBuffer* Buffer, const RHIBufferSRVCreateInfo& Desc)
{
	return DynamicPtrCast(std::make_shared<VulkanShaderResourceView>(Device, Buffer, Desc), RHIShaderResourceView);
}

RHIUnorderedAccessViewSP VulkanRHIApi::CreateBufferUnorderedAccessView(
	RHIBuffer* Buffer, const RHIBufferUAVCreateInfo& Desc)
{
	return DynamicPtrCast(std::make_shared<VulkanUnorderedAccessView>(Device, Buffer, Desc), RHIUnorderedAccessView);
}

// 创建 StagingBuffer
RHIStagingBufferSP VulkanRHIApi::CreateStagingBuffer(uint32_t size)
{
	return DynamicPtrCast(Device->GetStagingManager()->Acquire(size), RHIStagingBuffer) ;
}

// 管线状态相关接口实现
RHIGraphicsPipelineStateSP VulkanRHIApi::CreateGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc)
{
    // 创建Vulkan图形管线状态
    return DynamicPtrCast(std::make_shared<VulkanGraphicsPipelineState>(Device, desc), RHIGraphicsPipelineState);; // 暂时返回nullptr
}

RHIComputePipelineStateSP VulkanRHIApi::CreateComputePipelineState(const RHIComputePipelineStateDesc& desc)
{
    // 创建Vulkan计算管线状态
    return DynamicPtrCast(std::make_shared<VulkanComputePipelineState>(Device, desc), RHIComputePipelineState);; // 暂时返回nullptr
}

RHIRayTracingPipelineStateSP VulkanRHIApi::CreateRayTracingsPipelineState(const RHIRayTracingPipelineStateDesc& desc)
{
    // 创建Vulkan光线追踪管线状态
    return nullptr; // 暂时返回nullptr
}

RHIVertexDescStateSP VulkanRHIApi::CreateVertexDescState(const RHIVertexDescStateDesc& desc)
{
    // 创建Vulkan顶点描述状态
    return DynamicPtrCast(std::make_shared<VulkanVertexDescState>(Device, desc), RHIVertexDescState);; // 暂时返回nullptr
}

RHIRasterizerStateSP VulkanRHIApi::CreateRasterizerState(const RHIRasterizerStateDesc& desc)
{
    // 创建Vulkan光栅化状态
    return DynamicPtrCast(std::make_shared<VulkanRasterizerState>(Device, desc), RHIRasterizerState);; // 暂时返回nullptr
}

RHIColorBlendStateSP VulkanRHIApi::CreateColorBlendState(const RHIColorBlendStateDesc& desc)
{
    // 创建Vulkan颜色混合状态
    return DynamicPtrCast(std::make_shared<VulkanColorBlendState>(Device, desc), RHIColorBlendState); // 暂时返回nullptr
}

RHIDepthStencilStateSP VulkanRHIApi::CreateDepthStencilState(const RHIDepthStencilStateDesc& desc)
{
    // 创建Vulkan深度模板状态
    return DynamicPtrCast(std::make_shared<VulkanDepthStencilState>(Device, desc), RHIDepthStencilState);; // 暂时返回nullptr
}

RHIVertexShaderSP VulkanRHIApi::CreateVertexShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan顶点着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIVertexShader>(shaderSourceCode), RHIVertexShader); // 暂时返回nullptr
}

RHIFragmentShaderSP VulkanRHIApi::CreateFragmentShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan片段着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIFragmentShader>(shaderSourceCode), RHIFragmentShader); // 暂时返回nullptr
}

RHIComputeShaderSP VulkanRHIApi::CreateComputeShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan计算着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIComputeShader>(shaderSourceCode), RHIComputeShader); // 暂时返回nullptr
}

RHIGeometryShaderSP VulkanRHIApi::CreateGeometryShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan几何着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIGeometryShader>(shaderSourceCode), RHIGeometryShader); // 暂时返回nullptr
}

RHITessControlShaderSP VulkanRHIApi::CreateTessControlShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan细分控制着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHITessControlShader>(shaderSourceCode), RHITessControlShader); // 暂时返回nullptr
}

RHITessEvalShaderSP VulkanRHIApi::CreateTessEvalShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan细分评估着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHITessEvalShader>(shaderSourceCode), RHITessEvalShader); // 暂时返回nullptr
}

RHIMeshShaderSP VulkanRHIApi::CreateMeshShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan网格着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIMeshShader>(shaderSourceCode), RHIMeshShader); // 暂时返回nullptr
}

RHITaskShaderSP VulkanRHIApi::CreateTaskShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan任务着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHITaskShader>(shaderSourceCode), RHITaskShader); // 暂时返回nullptr
}

RHIRayGenShaderSP VulkanRHIApi::CreateRayGenShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan光线生成着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIRayGenShader>(shaderSourceCode), RHIRayGenShader); // 暂时返回nullptr
}

RHICloseHitShaderSP VulkanRHIApi::CreateCloseHitShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan最近命中着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHICloseHitShader>(shaderSourceCode), RHICloseHitShader); // 暂时返回nullptr
}

RHIMissShaderSP VulkanRHIApi::CreateMissShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan未命中着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIMissShader>(shaderSourceCode), RHIMissShader); // 暂时返回nullptr
}

RHIAnyHitShaderSP VulkanRHIApi::CreateAnyHitShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan任何命中着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIAnyHitShader>(shaderSourceCode), RHIAnyHitShader); // 暂时返回nullptr
}

RHIIntersectionShaderSP VulkanRHIApi::CreateIntersectionShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan相交着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHIIntersectionShader>(shaderSourceCode), RHIIntersectionShader); // 暂时返回nullptr
}

RHICallableShaderSP VulkanRHIApi::CreateCallableShader(const std::vector<char>& shaderSourceCode)
{
    // 创建Vulkan可调用着色器
    return DynamicPtrCast(Device->GetShaderManager()->GetOrCreateShader<VulkanRHICallableShader>(shaderSourceCode), RHICallableShader); // 暂时返回nullptr
}


RHISwapchainSP VulkanRHIApi::CreateSwapchain(void* inWindowHandle, uint32_t w, uint32_t h, ERHIFormat format)
{
	return std::make_shared<VulkanRHISwapchain>(Device, w, h, inWindowHandle, format);
}

RHISamplerSP VulkanRHIApi::CreateSampler(const RHISamplerDesc& desc)
{
    // 创建Vulkan采样器
    return std::make_shared<VulkanSampler>(Device,desc); // 暂时返回nullptr
}

RHIQueue* VulkanRHIApi::GetQueue(EQueueType Type)
{
	switch (Type)
	{
	case EQueueType::Graphics:
		return Device ? Device->GetGraphicsQueue() : nullptr;
	case EQueueType::Compute:
		return Device ? (Device->GetComputeQueue() ? Device->GetComputeQueue() : Device->GetGraphicsQueue()) : nullptr;
	case EQueueType::Transfer:
		if (!Device)
		{
			return nullptr;
		}
		if (Device->GetTransferQueue())
		{
			return Device->GetTransferQueue();
		}
		if (Device->GetComputeQueue())
		{
			return Device->GetComputeQueue();
		}
		return Device->GetGraphicsQueue();
	default:
		return nullptr;
	}
}

RHIPresentExecutor* VulkanRHIApi::GetPresentExecutor()
{
	return Device->GetPresentExecutor();
}

void VulkanRHIApi::RHICreateTransition(RHITransition* Transition, const RHITransitionCreateInfo& CreateInfo)
{
    

    if (!Transition)
        return;

    VulkanPipelineBarrier* pipelineBarrier = Transition->GetPrivateData<VulkanPipelineBarrier>();
    if (!pipelineBarrier)
        return;

    // placement new 确保私有数据对象被构造
    new (pipelineBarrier) VulkanPipelineBarrier();

	auto ResolveQueueFamilyIndex = [this](EQueueType queueType) -> uint32_t
	{
		if (!Device)
		{
			return VK_QUEUE_FAMILY_IGNORED;
		}
		switch (queueType)
		{
		case EQueueType::Graphics:
			return Device->GetGraphicsQueue() ? Device->GetGraphicsQueue()->GetFamilyIndex() : VK_QUEUE_FAMILY_IGNORED;
		case EQueueType::Compute:
			if (Device->GetComputeQueue())
			{
				return Device->GetComputeQueue()->GetFamilyIndex();
			}
			return Device->GetGraphicsQueue() ? Device->GetGraphicsQueue()->GetFamilyIndex() : VK_QUEUE_FAMILY_IGNORED;
		case EQueueType::Transfer:
			if (Device->GetTransferQueue())
			{
				return Device->GetTransferQueue()->GetFamilyIndex();
			}
			if (Device->GetComputeQueue())
			{
				return Device->GetComputeQueue()->GetFamilyIndex();
			}
			return Device->GetGraphicsQueue() ? Device->GetGraphicsQueue()->GetFamilyIndex() : VK_QUEUE_FAMILY_IGNORED;
		default:
			return VK_QUEUE_FAMILY_IGNORED;
		}
	};

	auto ResolveQueueTypeFromAccess = [](ERHIResourceAccess access) -> EQueueType
	{
		if (access == ERHIResourceAccess::Unknown || access == ERHIResourceAccess::Undefined)
		{
			return EQueueType::Graphics;
		}

		const ERHIResourceAccessFlags accessFlags(access);

		const bool hasCompute =
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::SRVCompute) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::UAVCompute);

		const bool hasGraphics =
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::SRVGraphics) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::UAVGraphics) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::RenderTargetView) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::DSVRead) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::DSVWrite) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::Present) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::VertexOrIndexBuffer) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::IndirectArgs);

		const bool hasTransfer =
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::CopySrc) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::CopyDest) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::ResolveSrc) ||
			EnumHasAnyFlags(accessFlags, ERHIResourceAccess::ResolveDst);

		if (hasTransfer && !hasCompute && !hasGraphics)
		{
			return EQueueType::Transfer;
		}

		if (hasCompute && !hasGraphics)
		{
			return EQueueType::Compute;
		}

		return EQueueType::Graphics;
	};

    for (const auto& transitionInfo : CreateInfo.TransitionInfos)
    {
        if (transitionInfo.Type == RHITransitionInfo::EType::Texture)
        {
            VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(transitionInfo.Texture);
            if (!vulkanTexture)
                continue;
            ERHIResourceAccess accessBefore = transitionInfo.AccessBefore;
            ERHIResourceAccess accessAfter = transitionInfo.AccessAfter;

            VkImageLayout oldLayout = DetermineImageLayout(accessBefore);
            VkImageLayout newLayout = DetermineImageLayout(accessAfter);

            if (accessBefore == accessAfter)
            {
                continue;
            }

			const EQueueType srcQueueType = ResolveQueueTypeFromAccess(accessBefore);
			const EQueueType dstQueueType = ResolveQueueTypeFromAccess(accessAfter);
			const uint32_t srcQueueFamilyIndex = ResolveQueueFamilyIndex(srcQueueType);
			const uint32_t dstQueueFamilyIndex = ResolveQueueFamilyIndex(dstQueueType);
			const bool crossQueueOwnership = srcQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED
				&& dstQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED
				&& srcQueueFamilyIndex != dstQueueFamilyIndex;
			const uint32_t barrierSrcQueueFamily = crossQueueOwnership ? srcQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED;
			const uint32_t barrierDstQueueFamily = crossQueueOwnership ? dstQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED;

			auto textureDesc = vulkanTexture->GetDesc();
			VkImageSubresourceRange subresourceRange{};
            subresourceRange.aspectMask = vulkanTexture->GetAspectFlags();
            subresourceRange.baseMipLevel = transitionInfo.MipIndex != RHISubresourceRange::kAllSubresources ? transitionInfo.MipIndex : 0;
            subresourceRange.levelCount = transitionInfo.MipIndex != RHISubresourceRange::kAllSubresources ? 1 : vulkanTexture->GetDesc().MipLevels;
            subresourceRange.baseArrayLayer = transitionInfo.ArraySlice != RHISubresourceRange::kAllSubresources ? transitionInfo.ArraySlice : 0;
            subresourceRange.layerCount = transitionInfo.ArraySlice != RHISubresourceRange::kAllSubresources ? 1 : vulkanTexture->GetDesc().ArraySize;

			pipelineBarrier->TransitionLayout(vulkanTexture->GetImage(), oldLayout, newLayout, subresourceRange, barrierSrcQueueFamily, barrierDstQueueFamily);
        }
		else if (transitionInfo.Type == RHITransitionInfo::EType::UAV) {
			VulkanUnorderedAccessView* vulkanUAV = static_cast<VulkanUnorderedAccessView*>(transitionInfo.UAV);
			if (!vulkanUAV) continue;

			if (vulkanUAV->IsTexture())
			{
				// 1. 从 UAV 句柄中提取底层的 Texture 指针
				// 注意：这里需要你的 UAV 类提供获取 Resource 的接口
				VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(vulkanUAV->GetResource());

				// 2. 提取 UAV 定义的子资源范围
				// 这一点非常重要！UAV 可能只覆盖了某一个 Mip 或 Layer
				VkImageSubresourceRange subresourceRange{};
				subresourceRange.aspectMask = vulkanTexture->GetAspectFlags();
				subresourceRange.baseMipLevel = vulkanUAV->GetBaseMipLevel();
				subresourceRange.levelCount = vulkanUAV->GetMipLevelCount();
				subresourceRange.baseArrayLayer = vulkanUAV->GetBaseArrayLayer();
				subresourceRange.layerCount = vulkanUAV->GetLayerCount();

				const EQueueType srcQueueType = ResolveQueueTypeFromAccess(transitionInfo.AccessBefore);
				const EQueueType dstQueueType = ResolveQueueTypeFromAccess(transitionInfo.AccessAfter);
				const uint32_t srcQueueFamilyIndex = ResolveQueueFamilyIndex(srcQueueType);
				const uint32_t dstQueueFamilyIndex = ResolveQueueFamilyIndex(dstQueueType);
				const bool crossQueueOwnership = srcQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED
					&& dstQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED
					&& srcQueueFamilyIndex != dstQueueFamilyIndex;
				const uint32_t barrierSrcQueueFamily = crossQueueOwnership ? srcQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED;
				const uint32_t barrierDstQueueFamily = crossQueueOwnership ? dstQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED;

				pipelineBarrier->TransitionAccess(vulkanTexture->GetImage(), transitionInfo.AccessBefore, transitionInfo.AccessAfter, subresourceRange, barrierSrcQueueFamily, barrierDstQueueFamily);

			}
			else if (vulkanUAV->IsBuffer())
			{
				VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(vulkanUAV->GetResource());

				// Buffer 不需要 Layout，直接记录 Access 变化用于生成 VkBufferMemoryBarrier

			}
		}
        else if (transitionInfo.Type == RHITransitionInfo::EType::Buffer)
        {
            // VulkanPipelineBarrier 当前仅支持图像屏障；Buffer 屏障可在后续补全。
        }
    }

    // 不在这里执行命令缓冲区屏障，交给命令上下文（Begin/End）执行。
}


void VulkanRHIApi::RHIReleaseTransition(RHITransition* Transition)
{
    if (!Transition)
        return;

    VulkanPipelineBarrier* pipelineBarrier = Transition->GetPrivateData<VulkanPipelineBarrier>();
    if (pipelineBarrier)
    {
        pipelineBarrier->~VulkanPipelineBarrier();
    }
}

// 创建上下文接口实现
RHITransientResourceManagerSP VulkanRHIApi::CreateTransientResourceManager()
{
	return std::make_shared<VulkanTransientResourceManager>(Device);
}

VkPhysicalDevice VulkanRHIApi::PickPhysicalDevice() {
	// 选择合适的物理设备
	uint32_t deviceCount = 0;
	VKFunc::EnumeratePhysicalDevices(Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		std::cerr << "No Vulkan-compatible devices found!" << std::endl;
		return VK_NULL_HANDLE;
	}
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	VKFunc::EnumeratePhysicalDevices(Instance, &deviceCount, physicalDevices.data());
	// 这里可以添加更多的逻辑来选择最合适的物理设备
	// 例如检查设备的特性、支持的扩展等
	auto preferredVendor = GetPreferredVendorId();
	for (const auto & device : physicalDevices) {
		VkPhysicalDeviceProperties deviceProperties;
		VKFunc::GetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		VKFunc::GetPhysicalDeviceFeatures(device, &deviceFeatures);
		std::cout << "Found device: " << deviceProperties.deviceName << std::endl;
		// 这里可以添加更多的检查逻辑
        if (GetVendorIdFromUint32(deviceProperties.vendorID) == preferredVendor) {
			return device; // 返回第一个符合条件的设备
        }
	}
    return nullptr;


}


std::vector<const char*> VulkanRHIApi::GetWantedInstanceLayers() {
	std::vector<const char*> wantLayers = VulkanPlatformSupport::GetPlatformWantedLayers();
    std::vector<const char*> res;
    //检查是否支持
	uint32_t layerCount = 0;
	VKFunc::EnumerateInstanceLayerProperties(&layerCount, nullptr);
	if (layerCount > 0) {
		std::vector<VkLayerProperties> availableLayers(layerCount);
		VKFunc::EnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (const auto& layer : availableLayers) {
			for (const auto& wantLayer : wantLayers) {
				if (strcmp(layer.layerName, wantLayer) == 0) {
                    res.push_back(wantLayer);
				}
			}
		}
	}
    return res;
}

std::vector<const char*> VulkanRHIApi::GetWantedInstanceExtensions() {
	std::vector<const char*> wantExtensions = VulkanPlatformSupport::GetPlatformWantedExtentions();
    std::vector<const char*> res;
	// 检查是否支持
	uint32_t extensionCount = 0;
	VKFunc::EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (extensionCount > 0) {
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		VKFunc::EnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
		for (const auto& extension : availableExtensions) {
			for (const auto& wantExtension : wantExtensions) {
				if (strcmp(extension.extensionName, wantExtension) == 0) {
					res.push_back(wantExtension);
				}
			}
		}
	}
    return res;
}

std::vector<const char*> VulkanRHIApi::GetWantedDeviceLayers() {
	std::vector<const char*> wantLayers = {
		"VK_LAYER_KHRONOS_validation" // Vulkan验证层
	};
	std::vector<const char*> res;
	// 检查是否支持
	uint32_t layerCount = 0;
	VKFunc::EnumerateDeviceLayerProperties(PhysicalDevice, &layerCount, nullptr);
	if (layerCount > 0) {
		std::vector<VkLayerProperties> availableLayers(layerCount);
		VKFunc::EnumerateDeviceLayerProperties(PhysicalDevice, &layerCount, availableLayers.data());
		for (const auto& layer : availableLayers) {
			for (const auto& wantLayer : wantLayers) {
				if (strcmp(layer.layerName, wantLayer) == 0) {
					res.push_back(wantLayer);
				}
			}
		}
	}
	return res;
}

std::vector<const char*> VulkanRHIApi::GetWantedDeviceExtensions() {
	std::vector<const char*> wantExtensions = VulkanPlatformSupport::GetPlatformWantedDeviceExtentions();
	wantExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // 交换链扩展
    std::vector<const char*> res;
	// 检查是否支持
	uint32_t extensionCount = 0;
	VKFunc::EnumerateDeviceExtensionProperties(PhysicalDevice,VK_NULL_HANDLE, &extensionCount, nullptr);
	if (extensionCount > 0) {
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		VKFunc::EnumerateDeviceExtensionProperties(PhysicalDevice, VK_NULL_HANDLE, &extensionCount, availableExtensions.data());
		for (const auto& extension : availableExtensions) {
			for (const auto& wantExtension : wantExtensions) {
				if (strcmp(extension.extensionName, wantExtension) == 0) {
					res.push_back(wantExtension);
				}
			}
		}
	}
	return res;
}







VulkanRHIModule::VulkanRHIModule() {

}
VulkanRHIModule::~VulkanRHIModule() {
}
void VulkanRHIModule::StartupModule()
{
	RHI::GRHIApi = CreateRHIApi();
	RHI::GRHIApi->Init();
	bLoaded = true;
}

void VulkanRHIModule::ShutdownModule()
{
	bLoaded = false;
}

bool VulkanRHIModule::IsLoaded() const
{
	return bLoaded;
}

RHIApi* VulkanRHIModule::CreateRHIApi()
{
	// Module 只负责创建，不负责生命周期
	return new VulkanRHIApi();
}

IMPLEMENT_SIMPLE_MODULE(VulkanRHIModule, "RHIVulkan");


}