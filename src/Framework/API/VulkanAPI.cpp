#include "VulkanAPI.h"


std::vector<VkPhysicalDevice> VulkanAPI::EnumeratePhysicalDevice(VkInstance instance)
{
	std::vector<VkPhysicalDevice> validPhysicalDevices{};
	uint32_t validPhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &validPhysicalDeviceCount, nullptr);
	validPhysicalDevices.resize(validPhysicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &validPhysicalDeviceCount, validPhysicalDevices.data());
	
	LogFunc(validPhysicalDeviceCount);
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
	
	LogFunc(validePhyscailDeviceProperties.size());

	return validePhyscailDeviceProperties;
}

std::vector<VkQueueFamilyProperties> VulkanAPI::GetQueueFamilyProperties(VkPhysicalDevice physicalDevice)
{
	std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	queueFamilyProperties.resize(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

	LogFunc(queueFamilyPropertiesCount);
	return queueFamilyProperties;
}

VkPhysicalDeviceFeatures VulkanAPI::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	return features;
}

VkDevice VulkanAPI::CreateDevice(VkPhysicalDevice physicalDevice, std::vector<std::pair<uint32_t, uint32_t>> wantQueueFamilyAndQueueCounts, std::vector<const char*> enableLayers, std::vector<const char*> enableExtensions, const VkPhysicalDeviceFeatures& enableFeatues)
{
	VkDevice device = VK_NULL_HANDLE;
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = wantQueueFamilyAndQueueCounts.size();
	std::vector<VkDeviceQueueCreateInfo> queueCreaetInfos(wantQueueFamilyAndQueueCounts.size());
	for (uint32_t i = 0; i < queueCreaetInfos.size(); i++)
	{
		VkDeviceQueueCreateInfo& queueCreateInfo = queueCreaetInfos[i];
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		float priority = 1.0f;
		queueCreateInfo.pQueuePriorities = &priority;
		queueCreateInfo.queueFamilyIndex = wantQueueFamilyAndQueueCounts[i].first;
		queueCreateInfo.queueCount = wantQueueFamilyAndQueueCounts[i].second;
	}

	deviceCreateInfo.pQueueCreateInfos = queueCreaetInfos.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
	deviceCreateInfo.ppEnabledLayerNames = enableLayers.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enableExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = enableExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &enableFeatues;
	vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	LogFunc(device);
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

VkSurfaceKHR VulkanAPI::CreateWin32Surface(VkInstance instance,GLFWwindow* window)
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	glfwCreateWindowSurface(instance, window, nullptr, &surface);
	LogFunc(surface);
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
	LogFunc(surfaceFormatsCount);
	return surfaceFormats;


}

VkSurfaceCapabilitiesKHR VulkanAPI::GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	return surfaceCapabilities;
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
	swapchainCreateInfoKHR.compositeAlpha = (VkCompositeAlphaFlagBitsKHR)0;//不执行任何混合
	swapchainCreateInfoKHR.presentMode = presentMode;
	swapchainCreateInfoKHR.clipped = VK_FALSE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;


	vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, nullptr, &swapchainKHR);
	LogFunc(swapchainKHR);
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
	LogFunc(imageCount);
	return images;
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
	vkCreateImage(device, &imageCreateInfo, nullptr, &image);
	LogFunc(image);

	return image;
}

VkImageView VulkanAPI::CreateImageView(VkDevice device, VkImageViewCreateFlags flags, VkImage image, VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange)
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
	vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
	LogFunc(imageView);

	return imageView;
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
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
	LogFunc(buffer);


	return buffer;
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
	vkCreateBufferView(device, &bufferViewCreateInfo, nullptr, &bufferView);
	LogFunc(bufferView);
	return bufferView;
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

	vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
	LogFunc(renderPass);

	return renderPass;
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
	vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffer);

	LogFunc(frameBuffer);

	return frameBuffer;
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
	vkCreateDescriptorSetLayout(device, &descritporSetLayerCreateInfo, nullptr, &descriptorSetLayout);
	LogFunc(descriptorSetLayout);

	return descriptorSetLayout;
}

VkPipelineLayout VulkanAPI::CreatePipelineLayout(VkDevice device, VkPipelineLayoutCreateFlags flags, const std::vector<VkDescriptorSetLayout> setLayouts, const std::vector<VkPushConstantRange> pushConstantRanges)
{
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutCreateInfo.pNext = setLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	LogFunc(pipelineLayout);


	return pipelineLayout;
}

VkPipeline VulkanAPI::CreateGraphicsPipeline(VkDevice device, VkPipelineCreateFlags flags, const std::vector<VkPipelineShaderStageCreateInfo> shaderStages, const VkPipelineVertexInputStateCreateInfo* vertexInputState, const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState, const VkPipelineTessellationStateCreateInfo* tessellationState, const VkPipelineViewportStateCreateInfo* viewportState, const VkPipelineRasterizationStateCreateInfo* rasterizationState, const VkPipelineMultisampleStateCreateInfo* multisampleState, const VkPipelineDepthStencilStateCreateInfo* depthStencilState, const VkPipelineColorBlendStateCreateInfo* colorBlendState, const VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, int32_t basePipelineIndex)
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
	graphicsPipelineCreateInfo.basePipelineHandle = basePipelineHandle;
	graphicsPipelineCreateInfo.basePipelineIndex = basePipelineIndex;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
	LogFunc(pipeline);



	return pipeline;
}



int32_t VulkanAPI::GetPhysicalDeviceSurportGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice)
{
	auto queueFamilyProperties = GetQueueFamilyProperties(physicalDevice);
	for (int32_t queueFamilyIndex = 0; queueFamilyIndex < static_cast<int32_t>(queueFamilyProperties.size()); queueFamilyIndex++)
	{
		if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			return queueFamilyIndex;
		}
	}
	return -1;
}

void* VulkanAPI::InstanceFuncLoader(VkInstance instance, const char* funcName)
{
	auto func = vkGetInstanceProcAddr(instance, funcName);
	LogFunc(func);
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
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "TestApplication";
	appInfo.pEngineName = "TestEngine";
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.ppEnabledExtensionNames = enableExtensions.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enableExtensions.size());
	instanceCreateInfo.ppEnabledLayerNames = enableLayers.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
	vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	LogFunc(instance);
	return instance;
}

void VulkanAPI::DestroyInstance(VkInstance instance)
{
	vkDestroyInstance(instance, nullptr);
}

void VulkanAPI::Init()
{
	glfwInit();
}

std::vector<const char*> VulkanAPI::GetWinGLFWExtensionNames()
{
	uint32_t extensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	std::vector<const char*> extensions{ glfwExtensions, glfwExtensions + extensionCount };
	return extensions;
}

GLFWwindow* VulkanAPI::CreateWin32Window(int width, int height, const char* windowName)
{
	auto window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	LogFunc(window);

	return window;
}

std::vector<VkLayerProperties> VulkanAPI::EnumerateInstanceSupportLayerProperties()
{
	
	std::vector<VkLayerProperties> supportLayes;
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	supportLayes.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, supportLayes.data());
	LogFunc(layerCount);
	return supportLayes;
}

std::vector<VkExtensionProperties> VulkanAPI::EnumerateInstanceSupportExtensionProperties()
{
	std::vector<VkExtensionProperties> extensionProperties;
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	extensionProperties.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
	LogFunc(extensionCount);
	return extensionProperties;
}

std::vector<VkExtensionProperties> VulkanAPI::EnumerateInstanceSupportLayerExtensionProperties( const char* layerName)
{
	std::vector<VkExtensionProperties> extensionProperties;
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);
	extensionProperties.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, extensionProperties.data());
	LogFunc(extensionCount);
	return extensionProperties;
}

VkBool32 DebugCallBack(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	LogFunc(1);
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
	debugUtilsMessagerCreateInfoEXT.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;//只看校验信息
	debugUtilsMessagerCreateInfoEXT.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;//只看错误信息
	debugUtilsMessagerCreateInfoEXT.pfnUserCallback = &DebugCallBack;
	
	((PFN_vkCreateDebugUtilsMessengerEXT)InstanceFuncLoader(instance, "vkCreateDebugUtilsMessengerEXT"))(instance,&debugUtilsMessagerCreateInfoEXT,nullptr,&debugUtilsMessenger);

	LogFunc(debugUtilsMessenger);
	return debugUtilsMessenger;

}
