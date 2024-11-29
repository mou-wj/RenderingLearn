#include "VulkanAPI.h"


std::vector<VkPhysicalDevice> VulkanAPI::EnumeratePhysicalDevice(VkInstance instance)
{
	std::vector<VkPhysicalDevice> validPhysicalDevices{};
	uint32_t validPhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &validPhysicalDeviceCount, nullptr);
	validPhysicalDevices.resize(validPhysicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &validPhysicalDeviceCount, validPhysicalDevices.data());
	
	ASSERT(validPhysicalDeviceCount);
	return validPhysicalDevices;
}

std::vector<VkPhysicalDeviceProperties> VulkanAPI::EnumeratePhysicalDeviceProperties(VkInstance instance)
{
	auto physicalDevices = EnumeratePhysicalDevice(instance);
	std::vector<VkPhysicalDeviceProperties> validePhyscailDeviceProperties{};
	validePhyscailDeviceProperties.resize(physicalDevices.size());
	for (uint32_t i = 0; i < validePhyscailDeviceProperties.size(); i++)
	{
		vkGetPhysicalDeviceProperties(physicalDevices[i], validePhyscailDeviceProperties.data() + i);
	}
	
	ASSERT(validePhyscailDeviceProperties.size());

	return validePhyscailDeviceProperties;
}

std::vector<VkQueueFamilyProperties> VulkanAPI::GetQueueFamilyProperties(VkPhysicalDevice physicalDevice)
{
	std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	queueFamilyProperties.resize(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	ASSERT(queueFamilyPropertiesCount);
	return queueFamilyProperties;
}

bool VulkanAPI::GetQueueFamilySurfaceSupport(VkPhysicalDevice  physicalDevice,uint32_t queueFamilyIndex,VkSurfaceKHR surface)
{
	VkBool32 support = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &support);
	return support;
}

VkPhysicalDeviceFeatures VulkanAPI::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	return features;
}

VkDevice VulkanAPI::CreateDevice(VkPhysicalDevice physicalDevice,const std::vector<std::pair<uint32_t, std::vector<float>>>& wantQueueFamilyAndQueuePriorities, std::vector<const char*> enableLayers, std::vector<const char*> enableExtensions, const VkPhysicalDeviceFeatures& enableFeatues)
{
	VkDevice device = VK_NULL_HANDLE;
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = wantQueueFamilyAndQueuePriorities.size();
	std::vector<VkDeviceQueueCreateInfo> queueCreaetInfos(wantQueueFamilyAndQueuePriorities.size());
	for (uint32_t i = 0; i < queueCreaetInfos.size(); i++)
	{
		VkDeviceQueueCreateInfo& queueCreateInfo = queueCreaetInfos[i];
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.pQueuePriorities = wantQueueFamilyAndQueuePriorities[i].second.data();
		queueCreateInfo.queueFamilyIndex = wantQueueFamilyAndQueuePriorities[i].first;
		queueCreateInfo.queueCount = wantQueueFamilyAndQueuePriorities[i].second.size();
	}

	deviceCreateInfo.pQueueCreateInfos = queueCreaetInfos.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = enableLayers.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enableExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = enableExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &enableFeatues;
	auto res = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	ASSERT(device);
	ASSERT(res == VK_SUCCESS);
	return device;
}

void VulkanAPI::DestroyDevice(VkDevice device)
{
	vkDestroyDevice(device, nullptr);
}

VkQueue VulkanAPI::GetQueue(VkDevice device, uint32_t familyIndex, uint32_t queueIndex)
{
	VkQueue queue{};
	vkGetDeviceQueue(device, familyIndex, queueIndex,&queue);
	return queue;
}

void VulkanAPI::DeviceWaitIdle(VkDevice device)
{
	vkDeviceWaitIdle(device);
}

VkPhysicalDeviceMemoryProperties VulkanAPI::GetMemoryProperties(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	return deviceMemoryProperties;

}

VkFormatProperties VulkanAPI::GetFormatPropetirs(VkPhysicalDevice physicalDevice, VkFormat format)
{
	VkFormatProperties formatProperties{};
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);


	return formatProperties;
}

std::vector<VkExtensionProperties> VulkanAPI::EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,const char* layerName)
{
	uint32_t extensionCount = 0;
	std::vector<VkExtensionProperties> extensionProps;
	vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &extensionCount, nullptr);
	extensionProps.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &extensionCount, extensionProps.data());
	return extensionProps;


}

std::vector<VkLayerProperties> VulkanAPI::EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice)
{
	uint32_t layerCount = 0;
	std::vector<VkLayerProperties> layerPropetries;
	vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
	layerPropetries.resize(layerCount);
	vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, layerPropetries.data());


	return layerPropetries;
}

VkImageFormatProperties VulkanAPI::GetImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags)
{
	VkImageFormatProperties imageFormatProps;
	vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, &imageFormatProps);
	return imageFormatProps;
}


VkMemoryRequirements VulkanAPI::GetImageMemoryRequirments(VkDevice device, VkImage image)
{
	VkMemoryRequirements memoryRequiremens{};
	vkGetImageMemoryRequirements(device, image, &memoryRequiremens);
	return memoryRequiremens;
}

VkSubresourceLayout VulkanAPI::GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource& subresource)
{
	VkSubresourceLayout layout{};
	vkGetImageSubresourceLayout(device, image, &subresource, &layout);
	return layout;
}


VkSurfaceKHR VulkanAPI::CreateWin32Surface(VkInstance instance,GLFWwindow* window)
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	auto res = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	ASSERT(surface);
	ASSERT(res == VK_SUCCESS);
	return surface;
}

void VulkanAPI::DestroySurface(VkInstance instance, VkSurfaceKHR surface)
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

std::vector<VkSurfaceFormatKHR> VulkanAPI::GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t surfaceFormatsCount = 0;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, nullptr);
	surfaceFormats.resize(surfaceFormatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, surfaceFormats.data());
	ASSERT(surfaceFormatsCount);
	return surfaceFormats;


}

VkSurfaceCapabilitiesKHR VulkanAPI::GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	return surfaceCapabilities;
}

std::vector<VkPresentModeKHR> VulkanAPI::GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t presentModeCount = 0;
	std::vector<VkPresentModeKHR> presentModes;

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,&presentModeCount,nullptr);
	presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
	ASSERT(presentModes.size());
	return presentModes;








}



VkSwapchainKHR VulkanAPI::CreateSwapchain(VkDevice device, VkSurfaceKHR surface, VkFormat format, VkColorSpaceKHR colorSpace, VkExtent2D extent, uint32_t numLayers, uint32_t numMips, uint32_t imageCount, VkImageUsageFlags imageUsage, VkSharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices, VkPresentModeKHR presentMode)
{
	VkSwapchainKHR swapchainKHR = VK_NULL_HANDLE;
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR{};
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.pNext = nullptr;
	swapchainCreateInfoKHR.flags = 0;
	swapchainCreateInfoKHR.surface = surface;
	swapchainCreateInfoKHR.minImageCount = imageCount;
	swapchainCreateInfoKHR.imageFormat = format;
	swapchainCreateInfoKHR.imageColorSpace = colorSpace;
	swapchainCreateInfoKHR.imageExtent = extent;
	swapchainCreateInfoKHR.imageArrayLayers = numLayers;
	swapchainCreateInfoKHR.imageUsage = imageUsage;
	swapchainCreateInfoKHR.imageSharingMode = sharingMode;
	swapchainCreateInfoKHR.queueFamilyIndexCount = queueFamilyIndices.size();
	swapchainCreateInfoKHR.pQueueFamilyIndices = queueFamilyIndices.data();
	swapchainCreateInfoKHR.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;//不进行变换
	swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//不执行任何混合
	swapchainCreateInfoKHR.presentMode = presentMode;
	swapchainCreateInfoKHR.clipped = VK_FALSE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;
	auto res = vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, nullptr, &swapchainKHR);
	ASSERT(swapchainKHR);
	ASSERT(res == VK_SUCCESS);
	return swapchainKHR;
}

void VulkanAPI::DestroySwapchain(VkDevice device, VkSwapchainKHR swapchain)
{
	vkDestroySwapchainKHR(device,swapchain,nullptr);
}

std::vector<VkImage> VulkanAPI::GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain)
{
	std::vector<VkImage> images{};
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
	ASSERT(imageCount);
	return images;
}

uint32_t VulkanAPI::GetNextValidSwapchainImageIndex(VkDevice device, VkSwapchainKHR swapchain, VkSemaphore  semaphore, VkFence  fence)
{
	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, fence, &imageIndex);
	return imageIndex;
}

VkDeviceMemory VulkanAPI::AllocateMemory(VkDevice device, VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
{
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = allocationSize;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	auto res = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
	ASSERT(deviceMemory);
	ASSERT(res == VK_SUCCESS);
	//std::cout << "currentAllocatedMemory : " << std::hex << deviceMemory << std::endl;
	return deviceMemory;
}

void VulkanAPI::ReleaseMemory(VkDevice device, VkDeviceMemory deviceMemory)
{
	vkFreeMemory(device, deviceMemory, nullptr);
	//std::cout << "releasedAllocatedMemory : " << std::hex << deviceMemory << std::endl;
}

void* VulkanAPI::MapMemory(VkDevice device, VkDeviceMemory deviceMemory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags mapFlags)
{
	void* p = nullptr;
	auto res = vkMapMemory(device, deviceMemory, offset, size, mapFlags, &p);
	ASSERT(p);
	ASSERT(res == VK_SUCCESS);
	return p;
}

void VulkanAPI::UnmapMemory(VkDevice device, VkDeviceMemory deviceMemory)
{
	vkUnmapMemory(device, deviceMemory);
}

void VulkanAPI::BindMemoryToImage(VkDevice device, VkDeviceMemory deviceMemory, VkImage image, VkDeviceSize offset)
{
	auto result = vkBindImageMemory(device, image, deviceMemory, offset);
	ASSERT(result == VK_SUCCESS);

}

void VulkanAPI::BindMemoryToBuffer(VkDevice device, VkDeviceMemory deviceMemory, VkBuffer buffer, VkDeviceSize offset)
{
	vkBindBufferMemory(device, buffer, deviceMemory, offset);
}


VkImage VulkanAPI::CreateImage(VkDevice device, VkImageCreateFlags flags, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices, VkImageLayout initialLayout)
{
	VkImage image = VK_NULL_HANDLE;
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = flags;
	imageCreateInfo.imageType = imageType;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = extent;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = arrayLayers;
	imageCreateInfo.samples = samples;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = sharingMode;
	imageCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
	imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	imageCreateInfo.initialLayout = initialLayout;
	auto res = vkCreateImage(device, &imageCreateInfo, nullptr, &image);
	ASSERT(res == VK_SUCCESS);
	ASSERT(image);

	return image;
}

void VulkanAPI::DestroyImage(VkDevice device, VkImage image)
{
	vkDestroyImage(device, image, nullptr);
}

VkImageView VulkanAPI::CreateImageView(VkDevice device, VkImageViewCreateFlags flags, VkImage image, VkImageUsageFlags usage,VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange)
{
	VkImageView imageView{};
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = flags;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = viewType;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = components;
	imageViewCreateInfo.subresourceRange = subresourceRange;
	VkImageViewUsageCreateInfo usageInfo{};
	usageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
	usageInfo.pNext = nullptr;
	usageInfo.usage = usage;
	auto res = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
	ASSERT(imageView);
	ASSERT(res == VK_SUCCESS);
	return imageView;
}

void VulkanAPI::DestroyImageView(VkDevice device, VkImageView imageView)
{
	vkDestroyImageView(device, imageView, nullptr);
}

VkBuffer VulkanAPI::CreateBuffer(VkDevice device, VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices)
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = flags;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = sharingMode;
	bufferCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
	bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	auto res = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
	ASSERT(buffer);
	ASSERT(res == VK_SUCCESS);

	return buffer;
}

void VulkanAPI::DestroyBuffer(VkDevice device, VkBuffer buffer)
{
	vkDestroyBuffer(device, buffer, nullptr);
}

VkMemoryRequirements VulkanAPI::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer)
{
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(device, buffer, &requirements);

	return requirements;
}

VkBufferView VulkanAPI::CreateBufferView(VkDevice device, VkBufferViewCreateFlags flags, VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range)
{
	VkBufferView bufferView = VK_NULL_HANDLE;
	VkBufferViewCreateInfo bufferViewCreateInfo{};
	bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	bufferViewCreateInfo.pNext = nullptr;
	bufferViewCreateInfo.flags = flags;
	bufferViewCreateInfo.buffer = buffer;
	bufferViewCreateInfo.format = format;
	bufferViewCreateInfo.offset = offset;
	bufferViewCreateInfo.range = range;
	auto res = vkCreateBufferView(device, &bufferViewCreateInfo, nullptr, &bufferView);
	ASSERT(bufferView);
	ASSERT(res == VK_SUCCESS);
	return bufferView;
}

void VulkanAPI::DestroyBufferView(VkDevice device, VkBufferView bufferView)
{
	vkDestroyBufferView(device, bufferView, nullptr);
}

VkRenderPass VulkanAPI::CreateRenderPass(VkDevice device, VkRenderPassCreateFlags flags, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency> dependencies)
{
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = flags;
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = subpasses.size();
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = dependencies.size();
	renderPassCreateInfo.pDependencies = dependencies.data();

	auto res = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
	ASSERT(renderPass);
	ASSERT(res == VK_SUCCESS);
	return renderPass;
}

void VulkanAPI::DestroyRenderPass(VkDevice device, VkRenderPass renderPass)
{
	vkDestroyRenderPass(device, renderPass, nullptr);
}


VkFramebuffer VulkanAPI::CreateFrameBuffer(VkDevice device, VkFramebufferCreateFlags flags, VkRenderPass renderPass, std::vector<VkImageView> attachments, uint32_t width, uint32_t height, uint32_t layers)
{
	VkFramebufferCreateInfo frameBufferCreateInfo{};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = nullptr;
	frameBufferCreateInfo.flags = flags;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = attachments.size();
	frameBufferCreateInfo.pAttachments = attachments.data();
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = layers;
	VkFramebuffer frameBuffer = VK_NULL_HANDLE;
	auto res = vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffer);

	ASSERT(frameBuffer);
	ASSERT(res == VK_SUCCESS);
	return frameBuffer;
}

void VulkanAPI::DestroyFrameBuffer(VkDevice device, VkFramebuffer frameBuffer)
{
	vkDestroyFramebuffer(device, frameBuffer, nullptr);
}



VkDescriptorSetLayout VulkanAPI::CreateDescriptorSetLayout(VkDevice device, VkDescriptorSetLayoutCreateFlags flags, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayoutCreateInfo descritporSetLayerCreateInfo{};
	descritporSetLayerCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descritporSetLayerCreateInfo.pNext = nullptr;
	descritporSetLayerCreateInfo.flags = flags;
	descritporSetLayerCreateInfo.bindingCount = bindings.size();
	descritporSetLayerCreateInfo.pBindings = bindings.data();
	auto res = vkCreateDescriptorSetLayout(device, &descritporSetLayerCreateInfo, nullptr, &descriptorSetLayout);
	ASSERT(descriptorSetLayout);
	ASSERT(res == VK_SUCCESS);
	return descriptorSetLayout;
}

void VulkanAPI::DestroyDesctriptorSetLayout(VkDevice device, VkDescriptorSetLayout desctriptorSetLayout)
{
	vkDestroyDescriptorSetLayout(device, desctriptorSetLayout, nullptr);
}

VkSampler VulkanAPI::CreateSampler(VkDevice device, VkSamplerCreateFlags flags, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW, float mipLodBias, VkBool32 anisotropyEnable, float maxAnisotropy, VkBool32 compareEnable, VkCompareOp compareOp, float minLod, float maxLod, VkBorderColor borderColor, VkBool32 unnormalizedCoordinates)
{
	VkSampler sampler = VK_NULL_HANDLE;
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = flags;
	samplerCreateInfo.magFilter = magFilter;
	samplerCreateInfo.minFilter = minFilter;
	samplerCreateInfo.mipmapMode = mipmapMode;
	samplerCreateInfo.addressModeU = addressModeU;
	samplerCreateInfo.addressModeV = addressModeV;
	samplerCreateInfo.addressModeW = addressModeW;
	samplerCreateInfo.mipLodBias = mipLodBias;
	samplerCreateInfo.anisotropyEnable = anisotropyEnable;
	samplerCreateInfo.maxAnisotropy = maxAnisotropy;
	samplerCreateInfo.compareEnable = compareEnable;
	samplerCreateInfo.compareOp = compareOp;
	samplerCreateInfo.minLod = minLod;
	samplerCreateInfo.maxLod = maxLod;
	samplerCreateInfo.borderColor = borderColor;
	samplerCreateInfo.unnormalizedCoordinates = unnormalizedCoordinates;
	auto res = vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);
	ASSERT(sampler);
	ASSERT(res == VK_SUCCESS);
	return sampler;
}

VkSampler VulkanAPI::CreateDefaultSampler(VkDevice device, float maxLod)
{

	return CreateSampler(device, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0, VK_FALSE, 0, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0, maxLod, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_FALSE);
}

void VulkanAPI::DestroySampler(VkDevice device, VkSampler sampler)
{
	vkDestroySampler(device, sampler, nullptr);
}



VkPipelineLayout VulkanAPI::CreatePipelineLayout(VkDevice device, VkPipelineLayoutCreateFlags flags, const std::vector<VkDescriptorSetLayout> setLayouts, const std::vector<VkPushConstantRange> pushConstantRanges)
{
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
	auto res = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	ASSERT(pipelineLayout);
	ASSERT(res == VK_SUCCESS);

	return pipelineLayout;
}

void VulkanAPI::DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout)
{
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

VkShaderModule VulkanAPI::CreateShaderModule(VkDevice device, VkShaderModuleCreateFlags flags, const std::vector<uint32_t>& spirv_code)
{
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = flags;
	shaderModuleCreateInfo.codeSize = spirv_code.size() * 4;
	shaderModuleCreateInfo.pCode = spirv_code.data();

	auto res = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
	ASSERT(shaderModule);
	ASSERT(res == VK_SUCCESS);
	return shaderModule;
}

void VulkanAPI::DestroyShaderModule(VkDevice device, VkShaderModule shaderModule)
{
	vkDestroyShaderModule(device, shaderModule, nullptr);
}

VkPipeline VulkanAPI::CreateGraphicsPipeline(VkDevice device, VkPipelineCreateFlags flags, const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, const VkPipelineVertexInputStateCreateInfo* vertexInputState, const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState, const VkPipelineTessellationStateCreateInfo* tessellationState, const VkPipelineViewportStateCreateInfo* viewportState, const VkPipelineRasterizationStateCreateInfo* rasterizationState, const VkPipelineMultisampleStateCreateInfo* multisampleState, const VkPipelineDepthStencilStateCreateInfo* depthStencilState, const VkPipelineColorBlendStateCreateInfo* colorBlendState, const VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass)
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = nullptr;
	graphicsPipelineCreateInfo.flags = flags;
	graphicsPipelineCreateInfo.stageCount = shaderStages.size();
	graphicsPipelineCreateInfo.pStages = shaderStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = vertexInputState;
	graphicsPipelineCreateInfo.pInputAssemblyState = inputAssemblyState;
	graphicsPipelineCreateInfo.pTessellationState = tessellationState;
	graphicsPipelineCreateInfo.pViewportState = viewportState;
	graphicsPipelineCreateInfo.pRasterizationState = rasterizationState;
	graphicsPipelineCreateInfo.pMultisampleState = multisampleState;
	graphicsPipelineCreateInfo.pDepthStencilState = depthStencilState;
	graphicsPipelineCreateInfo.pColorBlendState = colorBlendState;
	graphicsPipelineCreateInfo.pDynamicState = dynamicState;
	graphicsPipelineCreateInfo.layout = layout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = subpass;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;
	auto res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	ASSERT(pipeline);
	ASSERT(res == VK_SUCCESS);


	return pipeline;
}

VkPipeline VulkanAPI::CreateComputePipeline(VkDevice device, VkPipelineCreateFlags flags, VkPipelineShaderStageCreateInfo stage, VkPipelineLayout layout)
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.flags = 0;
	computePipelineCreateInfo.stage = stage;
	computePipelineCreateInfo.layout = layout;
	computePipelineCreateInfo.basePipelineHandle = 0;
	computePipelineCreateInfo.basePipelineIndex = -1;
	auto res = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline);
	ASSERT(res == VK_SUCCESS);
	return pipeline;

}


void VulkanAPI::DestroyPipeline(VkDevice device, VkPipeline pipeline)
{
	vkDestroyPipeline(device, pipeline, nullptr);
}

VkDescriptorPool VulkanAPI::CreateDescriptorPool(VkDevice device, VkDescriptorPoolCreateFlags flags, uint32_t maxSets, const std::vector<VkDescriptorPoolSize> poolSizes)
{
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = flags;
	descriptorPoolCreateInfo.maxSets = maxSets;
	descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	auto res = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
	ASSERT(descriptorPool);
	ASSERT(res == VK_SUCCESS);
	return descriptorPool;
}

void VulkanAPI::DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool)
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

}

VkDescriptorSet VulkanAPI::AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> setLayouts)
{
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = setLayouts.size();
	descriptorSetAllocateInfo.pSetLayouts = setLayouts.data();
	auto res = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);
	ASSERT(descriptorSet);
	ASSERT(res == VK_SUCCESS);
	return descriptorSet;
}

void VulkanAPI::ReleaseDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSet>& descriptorSets)
{
	vkFreeDescriptorSets(device, descriptorPool, descriptorSets.size(), descriptorSets.data());
}



std::vector<VkCommandBuffer> VulkanAPI::AllocateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t commandBufferCount)
{
	std::vector<VkCommandBuffer> commandBuffers;
	commandBuffers.resize(commandBufferCount);
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
	commandBufferAllocateInfo.level = level;

	auto res = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data());
	ASSERT(res == VK_SUCCESS);
	return commandBuffers;

}

void VulkanAPI::ReleaseCommandBuffers(VkDevice device, VkCommandPool commandPool, const std::vector<VkCommandBuffer>& commandBuffers)
{
	vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
}

VkFence VulkanAPI::CreateFence(VkDevice device, VkFenceCreateFlags flags)
{
	VkFence fence = VK_NULL_HANDLE;
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = flags;
	auto res = vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
	ASSERT(fence);
	ASSERT(res == VK_SUCCESS);
	return fence;
}

void VulkanAPI::DestroyFence(VkDevice device, VkFence fence)
{
	vkDestroyFence(device, fence, nullptr);
}

VkResult VulkanAPI::WaitFence(VkDevice device,const std::vector<VkFence>& fences,bool waitAll)
{
	return vkWaitForFences(device, fences.size(), fences.data(), waitAll, UINT64_MAX);
}

void VulkanAPI::ResetFences(VkDevice device, const std::vector<VkFence>& fences)
{
	auto res = vkResetFences(device, fences.size(), fences.data());
	ASSERT(res == VK_SUCCESS);
}

VkResult VulkanAPI::GetFenceStatus(VkDevice device, VkFence fence)
{
	auto res = vkGetFenceStatus(device, fence);
	return res;
}

VkSemaphore VulkanAPI::CreateSemaphore(VkDevice device, VkSemaphoreCreateFlags flags)
{
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = flags;
	auto res = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
	ASSERT(semaphore);
	ASSERT(res == VK_SUCCESS);
	return semaphore;



}

void VulkanAPI::DestroySemaphore(VkDevice device, VkSemaphore semaphore)
{
	vkDestroySemaphore(device, semaphore, nullptr);
}


void VulkanAPI::WaitTimelineSemaphores(VkDevice device, VkSemaphoreWaitFlags flags,const std::vector<VkSemaphore>& semaphores, const std::vector<uint64_t>& semaphoreValues)
{
	VkSemaphoreWaitInfo waitInfo{ };
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.pNext = nullptr;
	waitInfo.flags = flags;
	waitInfo.semaphoreCount = semaphores.size();
	waitInfo.pSemaphores = semaphores.data();
	waitInfo.pValues = semaphoreValues.data();
	auto res = vkWaitSemaphores(device, &waitInfo, VK_TIMEOUT);
}

void VulkanAPI::SignalSemaphores(VkDevice device, VkSemaphore semaphore, uint64_t value/*只对timeline semaphore有用*/)
{

	VkSemaphoreSignalInfo signalInfo{};
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	signalInfo.pNext = nullptr;
	signalInfo.semaphore = semaphore;
	signalInfo.value = value;
	vkSignalSemaphore(device, &signalInfo);

}


VkEvent VulkanAPI::CreateEvent(VkDevice device, VkEventCreateFlags flags)
{
	VkEvent c_event = VK_NULL_HANDLE;
	VkEventCreateInfo eventCreateInfo{};
	eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	eventCreateInfo.pNext = nullptr;
	eventCreateInfo.flags = flags;
	auto res = vkCreateEvent(device, &eventCreateInfo, nullptr, &c_event);
	ASSERT(c_event);
	ASSERT(res == VK_SUCCESS);
	return c_event;
}

void VulkanAPI::DestroyEvent(VkDevice device, VkEvent c_event)
{
	vkDestroyEvent(device, c_event, nullptr);
}

void VulkanAPI::CommandBufferReset(VkCommandBuffer commandBuffer)
{
	vkResetCommandBuffer(commandBuffer, 0);
}

void VulkanAPI::BeginRecord(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags  flags)
{
	VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;
	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = flags;
	vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo);


}

void VulkanAPI::EndRecord(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
}

void VulkanAPI::CmdBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, const std::vector<VkClearValue>& cleatValues, VkSubpassContents  subpassContents)
{
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderArea = renderArea;
	renderPassBeginInfo.clearValueCount = cleatValues.size();
	renderPassBeginInfo.pClearValues = cleatValues.data();
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, subpassContents);

}

void VulkanAPI::CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanAPI::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents  subpassContents)
{
	vkCmdNextSubpass(commandBuffer, subpassContents);
}

void VulkanAPI::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t queryIndex)
{
	vkCmdEndQuery(commandBuffer, queryPool, queryIndex);
}

void VulkanAPI::CmdResetQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQueryIndex,uint32_t numQuery)
{
	vkCmdResetQueryPool(commandBuffer,queryPool,firstQueryIndex,numQuery);
}

void VulkanAPI::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t queryIndex, VkQueryControlFlags flags)
{
	vkCmdBeginQuery(commandBuffer, queryPool, queryIndex, flags);
}

void VulkanAPI::CmdBeginRendering(VkCommandBuffer commandBuffer, VkRenderingFlags flags, VkRect2D renderArea, uint32_t layerCount, uint32_t viewMask, const std::vector<VkRenderingAttachmentInfo>& colorAttachments, const VkRenderingAttachmentInfo* depthAttachment, const VkRenderingAttachmentInfo* stencilAttachment)
{
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pNext = nullptr;
	renderingInfo.flags = flags;
	renderingInfo.renderArea = renderArea;
	renderingInfo.layerCount = layerCount;
	renderingInfo.viewMask = viewMask;
	renderingInfo.colorAttachmentCount = colorAttachments.size();
	renderingInfo.pColorAttachments = colorAttachments.data();
	renderingInfo.pDepthAttachment = depthAttachment;
	renderingInfo.pStencilAttachment = stencilAttachment;
	vkCmdBeginRendering(commandBuffer, &renderingInfo);

}

void VulkanAPI::CmdEndRendering(VkCommandBuffer commandBuffer)
{
	vkCmdEndRendering(commandBuffer);
}

void VulkanAPI::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets)
{
	ASSERT(offsets.size() >= buffers.size());
	vkCmdBindVertexBuffers(commandBuffer, firstBinding, buffers.size(), buffers.data(), offsets.data());
}

void VulkanAPI::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void VulkanAPI::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void VulkanAPI::CmdBindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, const std::vector<VkDescriptorSet> descriptorSets, const std::vector<uint32_t> dynamicOffsets)
{
	vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSets.size(), descriptorSets.data(), dynamicOffsets.size(), dynamicOffsets.size() ? dynamicOffsets.data() : nullptr);
}



void VulkanAPI::CmdDynamicSetViewPorts(VkCommandBuffer commandBuffer, uint32_t firstViewport, const std::vector<VkViewport>& viewports)
{
	vkCmdSetViewport(commandBuffer, firstViewport, viewports.size(), viewports.data());
}

void VulkanAPI::CmdClearAttachments(VkCommandBuffer commandBuffer, const std::vector<VkClearAttachment>& clearAttachments, const std::vector<VkClearRect>& clearRegions)
{

	vkCmdClearAttachments(commandBuffer, clearAttachments.size(), clearAttachments.data(), clearRegions.size(), clearRegions.data());
}

void VulkanAPI::CmdClearDepthImage(VkCommandBuffer commandBuffer,VkImage depthImage,VkImageLayout imageLayout,const VkClearDepthStencilValue& clearValue,const std::vector<VkImageSubresourceRange>& clearRegions)
{
	vkCmdClearDepthStencilImage(commandBuffer, depthImage, imageLayout, &clearValue, clearRegions.size(), clearRegions.data());

}



void VulkanAPI::CmdDrawVertex(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanAPI::CmdDrawIndex(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanAPI::CmdCopyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageCopy>& copyRegions)
{

	vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, copyRegions.size(), copyRegions.data());
}

void VulkanAPI::CmdBlitImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageBlit> blitRegions, VkFilter filter)
{
	vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, blitRegions.size(), blitRegions.data(), filter);
}


void VulkanAPI::CmdMemoryBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier>& memoryBarriers, const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers, const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers)
{
	ASSERT(memoryBarriers.size() || bufferMemoryBarriers.size() || imageMemoryBarriers.size());
	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarriers.size(), memoryBarriers.data(), bufferMemoryBarriers.size(), bufferMemoryBarriers.data(), imageMemoryBarriers.size(), imageMemoryBarriers.data());
}

void VulkanAPI::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}





void VulkanAPI::SubmitCommands(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkPipelineStageFlags>& waitDstStageMask, const std::vector<VkCommandBuffer>& commandBuffers, const std::vector<VkSemaphore>& signalSemaphores, VkFence allCommandFinishedFence)
{
	ASSERT(waitDstStageMask.size() == waitSemaphores.size());
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.empty() ? VK_NULL_HANDLE : waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitSemaphores.empty() ? VK_NULL_HANDLE : waitDstStageMask.data();
	submitInfo.commandBufferCount = commandBuffers.size();
	submitInfo.pCommandBuffers = commandBuffers.empty() ? VK_NULL_HANDLE : commandBuffers.data();
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.empty() ? VK_NULL_HANDLE : signalSemaphores.data();

	SubmitCommands(queue, { submitInfo }, allCommandFinishedFence);

}

void VulkanAPI::SubmitCommands(VkQueue queue, const std::vector<VkSubmitInfo>& submitInfos, VkFence allCommandFinishedFence)
{
	auto result = vkQueueSubmit(queue, submitInfos.size(), submitInfos.data(), allCommandFinishedFence);
	if (result != VK_SUCCESS)
	{
		assert(0);
	}
}

void VulkanAPI::Present(VkQueue queue, const std::vector<VkSemaphore>& waitSemaphores , const std::vector<VkSwapchainKHR>& swapchains, const std::vector<uint32_t>& swapchainImageIndices, std::vector<VkResult>& outResults)
{
	ASSERT(swapchains.size() <= swapchainImageIndices.size());
	outResults.resize(swapchains.size());
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = waitSemaphores.size();
	presentInfo.pWaitSemaphores = waitSemaphores.empty() ? VK_NULL_HANDLE : waitSemaphores.data();
	presentInfo.swapchainCount = swapchains.size();
	presentInfo.pSwapchains = swapchains.empty() ? VK_NULL_HANDLE : swapchains.data();
	presentInfo.pImageIndices = swapchainImageIndices.data();
	presentInfo.pResults = outResults.empty() ? VK_NULL_HANDLE : outResults.data();
	vkQueuePresentKHR(queue, &presentInfo);
}

void VulkanAPI::WaitQueueIdle(VkQueue queue)
{
	vkQueueWaitIdle(queue);
}






void VulkanAPI::UpdateDescriptorSetBindingResources(VkDevice device, VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const std::vector<VkDescriptorImageInfo> imageInfos, const std::vector<VkDescriptorBufferInfo> bufferInfos, const std::vector<VkBufferView> texelBufferViews)
{
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = nullptr;
	writeDescriptorSet.dstSet = dstSet;
	writeDescriptorSet.dstBinding = dstBinding;
	writeDescriptorSet.dstArrayElement = dstArrayElement;
	writeDescriptorSet.descriptorCount = descriptorCount;
	writeDescriptorSet.descriptorType = descriptorType;
	writeDescriptorSet.pImageInfo = imageInfos.size() ? imageInfos.data() : nullptr;
	writeDescriptorSet.pBufferInfo = bufferInfos.size() ? bufferInfos.data() : nullptr;
	writeDescriptorSet.pTexelBufferView = texelBufferViews.size() ? texelBufferViews.data() : nullptr;
	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);



}

VkQueryPool VulkanAPI::CreateQueryPool(VkDevice device, VkQueryPoolCreateFlags  flags, VkQueryType  queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags  pipelineStatistics)
{
	VkQueryPool queryPool = VK_NULL_HANDLE;
	VkQueryPoolCreateInfo queryPoolCreateInfo{};
	queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolCreateInfo.pNext = nullptr;
	queryPoolCreateInfo.flags = flags;
	queryPoolCreateInfo.queryType = queryType;
	queryPoolCreateInfo.queryCount = queryCount;
	queryPoolCreateInfo.pipelineStatistics = pipelineStatistics;
	auto res = vkCreateQueryPool(device, &queryPoolCreateInfo, nullptr, &queryPool);
	ASSERT(res == VK_SUCCESS);
	ASSERT(queryPool);
	return queryPool;
}

void VulkanAPI::DestroyQueryPool(VkDevice device, VkQueryPool pool)
{
	vkDestroyQueryPool(device, pool, nullptr);
}

void VulkanAPI::ResetQueryPool(VkDevice device, VkQueryPool pool, uint32_t firstQuery, uint32_t numQuery)
{
	vkResetQueryPool(device, pool, firstQuery, numQuery);
}

std::vector<uint64_t> VulkanAPI::GetQueryResult(VkDevice device, VkQueryPool pool,uint32_t firstQuery,uint32_t numQuery,VkQueryResultFlags resultFlags)
{
	std::vector<uint64_t> res;
	res.resize(numQuery);
	vkGetQueryPoolResults(device, pool, firstQuery, numQuery, res.size() * sizeof(uint64_t), res.data(), sizeof(uint64_t), resultFlags);
	return res;
}

VkCommandPool VulkanAPI::CreateCommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex)
{
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = flags;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
	auto res = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
	ASSERT(commandPool);
	ASSERT(res == VK_SUCCESS);
	return commandPool;
}

void VulkanAPI::DesctroyCommandPool(VkDevice device, VkCommandPool commandPool)
{
	vkDestroyCommandPool(device, commandPool, nullptr);
}

VkCommandBuffer VulkanAPI::AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel  level)
{
	auto commandBuffers = AllocateCommandBuffers(device, commandPool, level, 1);
	ASSERT(commandBuffers.size());
	return commandBuffers[0];
}

void VulkanAPI::ReleaseCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
{
	ReleaseCommandBuffers(device, commandPool, { commandBuffer });
}

void* VulkanAPI::InstanceFuncLoader(VkInstance instance, const char* funcName)
{
	auto func = vkGetInstanceProcAddr(instance, funcName);
	ASSERT(func);
	return func;
}

VkInstance VulkanAPI::CreateInstance(std::vector<const char*> enableLayers, std::vector<const char*> enableExtensions)
{
	VkInstance instance = VK_NULL_HANDLE;
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;//不指定额外参数
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "TestApplication";
	appInfo.pEngineName = "TestEngine";
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.ppEnabledExtensionNames = enableExtensions.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enableExtensions.size());
	instanceCreateInfo.ppEnabledLayerNames = enableLayers.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
	auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	ASSERT(instance);
	ASSERT(res == VK_SUCCESS);
	return instance;
}

void VulkanAPI::DestroyInstance(VkInstance instance)
{
	vkDestroyInstance(instance, nullptr);
}

void VulkanAPI::Initialize()
{
	glfwInit();
}

std::vector<const char*> VulkanAPI::GetInstanceNeedWinGLFWExtensionNames()
{
	uint32_t extensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	std::vector<const char*> extensions{ glfwExtensions, glfwExtensions + extensionCount };
	return extensions;
}

GLFWwindow* VulkanAPI::CreateWin32Window(int width, int height, const char* windowName)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	auto window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	ASSERT(window);

	return window;
}

void VulkanAPI::DestroyGLFWWin32Window(GLFWwindow* window)
{
	glfwDestroyWindow(window);
}

std::vector<VkLayerProperties> VulkanAPI::EnumerateLayerProperties()
{
	
	std::vector<VkLayerProperties> supportLayes;
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	supportLayes.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, supportLayes.data());
	ASSERT(layerCount);
	return supportLayes;
}

std::vector<VkExtensionProperties> VulkanAPI::EnumerateExtensionProperties(const char* layerName)
{
	std::vector<VkExtensionProperties> extensionProperties;
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);
	extensionProperties.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, extensionProperties.data());
	ASSERT(extensionCount);
	return extensionProperties;
}


VkBool32 DebugCallBack(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	LogInfo(pCallbackData->pMessage);

	//ASSERT(1);
	return VK_TRUE;
}

VkDebugUtilsMessengerEXT VulkanAPI::CreateDebugInfoMessager(VkInstance instance)
{
	VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessagerCreateInfoEXT{};
	debugUtilsMessagerCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessagerCreateInfoEXT.pNext = nullptr;
	debugUtilsMessagerCreateInfoEXT.flags = 0;
	debugUtilsMessagerCreateInfoEXT.pUserData = nullptr;
	debugUtilsMessagerCreateInfoEXT.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;//只看校验信息	
	debugUtilsMessagerCreateInfoEXT.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;//只看错误信息
	debugUtilsMessagerCreateInfoEXT.pfnUserCallback = &DebugCallBack;
	
	auto res = ((PFN_vkCreateDebugUtilsMessengerEXT)InstanceFuncLoader(instance, "vkCreateDebugUtilsMessengerEXT"))(instance,&debugUtilsMessagerCreateInfoEXT,nullptr,&debugUtilsMessenger);

	ASSERT(debugUtilsMessenger);
	return debugUtilsMessenger;

}

void VulkanAPI::DestroyDebugInfoMessager(VkInstance instance, VkDebugUtilsMessengerEXT debugMessager)
{
	((PFN_vkDestroyDebugUtilsMessengerEXT)InstanceFuncLoader(instance, "vkDestroyDebugUtilsMessengerEXT"))(instance, debugMessager, nullptr);
}
