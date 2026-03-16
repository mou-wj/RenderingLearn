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

#define DynamicPtrCast(ptr, type) (std::dynamic_pointer_cast<type>(ptr))

namespace RHIVulkan{
    


// 析构函数实现
    VulkanRHIApi::~VulkanRHIApi()
{
    Shutdown();
}

// 初始化和销毁接口实现
bool VulkanRHIApi::Init()
{
	GShaderPlatform = ERHIShaderPlatform::Vulkan;
    // 创建Vulkan实例
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "WREngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "WREngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

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
    VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
    if (result != VK_SUCCESS)
    {
        // 处理错误
       
        return false;
    }

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

    return true;
    // 初始化其他Vulkan资源
    // 如物理设备、逻辑设备、队列等
}

void VulkanRHIApi::Shutdown()
{
    // 销毁Vulkan实例和其他资源
    if (Instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(Instance, nullptr);
        Instance = VK_NULL_HANDLE;
    }

    // 销毁其他Vulkan资源
}

RHIShaderLibrarySP VulkanRHIApi::CreateShaderLibrary(const std::string& name, ERHIShaderPlatform platform) {
	return nullptr; // 暂时返回nullptr
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




void VulkanRHIApi::UpdateTexture(RHICommandList& cmdList, RHITexture* texture, const void* data,const RHITextureRegion& region)
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
void VulkanRHIApi::UpdateBuffer(RHICommandList& cmdList, RHIBuffer* buffer, const void* data, const RHIBufferRegion& region)
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
	return nullptr; // 暂时返回nullptr
}

RHIUnorderedAccessViewSP VulkanRHIApi::CreateTextureUnorderedAccessView(
	RHITexture* Texture, const RHITexUAVCreateInfo& Desc)
{
	return nullptr; // 暂时返回nullptr
}

RHIShaderResourceViewSP VulkanRHIApi::CreateBufferShaderResourceView(
	RHIBuffer* Buffer, const RHIBufferSRVCreateInfo& Desc)
{
	return nullptr; // 暂时返回nullptr
}

RHIUnorderedAccessViewSP VulkanRHIApi::CreateBufferUnorderedAccessView(
	RHIBuffer* Buffer, const RHIBufferUAVCreateInfo& Desc)
{
	return nullptr; // 暂时返回nullptr
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
    return nullptr; // 暂时返回nullptr
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


RHIFenceSP VulkanRHIApi::CreateFence()
{
    // 创建Vulkan同步围栏
    return nullptr; // 暂时返回nullptr
}

RHIViewportSP VulkanRHIApi::CreateViewport(void* inWindowHandle, uint32_t w, uint32_t h, ERHIFormat format)
{
    // 创建Vulkan视口
    return std::make_shared<VulkanViewport>(Device,w,h,inWindowHandle,format); // 暂时返回nullptr
}

RHITextureSP VulkanRHIApi::GetViewportBackBuffer(RHIViewport* viewport)
{
	auto vulkanViewport = static_cast<VulkanViewport*>(viewport);
	return std::dynamic_pointer_cast<RHITexture>(vulkanViewport->GetBackTexture());
}

RHISamplerSP VulkanRHIApi::CreateSampler(const RHISamplerDesc& desc)
{
    // 创建Vulkan采样器
    return std::make_shared<VulkanSampler>(Device,desc); // 暂时返回nullptr
}

// 创建上下文接口实现
RHICommandContex* VulkanRHIApi::GetDefualtCommandContex()
{
    // 创建Vulkan图形上下文
    return Device->GetGlobalCommandContext(); // 暂时返回nullptr
}


RHITransientResourceManagerSP VulkanRHIApi::CreateTransientResourceManager()
{
	return std::make_shared<VulkanTransientResourceManager>(Device);
}

struct VulkanPlatformCommandList : public RHIPlatformCommandList {
		VulkanCommandContext* CommandContext;
};

RHIPlatformCommandList* VulkanRHIApi::FinalizeCommandContex(RHICommandContex* contex)
{
    // 创建Vulkan图形上下文
	VulkanPlatformCommandList* platformCommandList = new VulkanPlatformCommandList();
    platformCommandList->CommandContext = dynamic_cast<VulkanCommandContext*>(contex);
	return platformCommandList;
}

void VulkanRHIApi::SubmitPlatformCommandLists(std::vector<RHIPlatformCommandList*> cmdLists)
{
	for (auto& cmdList : cmdLists) {
		VulkanPlatformCommandList* vulkanCmdList = dynamic_cast<VulkanPlatformCommandList*>(cmdList);
		if (vulkanCmdList && vulkanCmdList->CommandContext) {
			vulkanCmdList->CommandContext->GetCommandBufferManager()->SubmitActiveCommandBuffer();
		}
		delete vulkanCmdList; // 提交后删除命令列表
	}
}


VkPhysicalDevice VulkanRHIApi::PickPhysicalDevice() {
	// 选择合适的物理设备
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		std::cerr << "No Vulkan-compatible devices found!" << std::endl;
		return VK_NULL_HANDLE;
	}
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(Instance, &deviceCount, physicalDevices.data());
	// 这里可以添加更多的逻辑来选择最合适的物理设备
	// 例如检查设备的特性、支持的扩展等
	auto preferredVendor = GetPreferredVendorId();
	for (const auto & device : physicalDevices) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
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
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	if (layerCount > 0) {
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
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
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (extensionCount > 0) {
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
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
	vkEnumerateDeviceLayerProperties(PhysicalDevice, &layerCount, nullptr);
	if (layerCount > 0) {
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateDeviceLayerProperties(PhysicalDevice, &layerCount, availableLayers.data());
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
	vkEnumerateDeviceExtensionProperties(PhysicalDevice,VK_NULL_HANDLE, &extensionCount, nullptr);
	if (extensionCount > 0) {
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, VK_NULL_HANDLE, &extensionCount, availableExtensions.data());
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