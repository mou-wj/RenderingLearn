#pragma once
#include "vulkan/vulkan.h"
#include "../Common/Log.h"
#include <GLFW/glfw3.h>
#include <vector>
namespace VulkanAPI {

	void Init();
	//platform
	std::vector<const char*> GetWinGLFWExtensionNames();
	GLFWwindow* CreateWin32Window(int width, int height, const char* windowName);


	//instance
	std::vector<VkLayerProperties> EnumerateInstanceSupportLayerProperties();
	std::vector<VkExtensionProperties> EnumerateInstanceSupportExtensionProperties();
	std::vector<VkExtensionProperties> EnumerateInstanceSupportLayerExtensionProperties(const char* layerName);
	VkInstance CreateInstance(std::vector<const char*> enableLayers,std::vector<const char*> enableExtensions);
	void DestroyInstance(VkInstance instance);
	void* InstanceFuncLoader(VkInstance instance, const char* funcName);
	VkDebugUtilsMessengerEXT CreateDebugInfoMessager(VkInstance instance);
	std::vector<VkPhysicalDevice> EnumeratePhysicalDevice(VkInstance instance);
	std::vector<VkPhysicalDeviceProperties> EnumeratePhysicalDeviceProperties(VkInstance instance);
	std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice);
	//physical device
	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);

	//device
	VkDevice CreateDevice(VkPhysicalDevice physicalDevice, std::vector<std::pair<uint32_t,uint32_t>> wantQueueFamilyAndQueueCounts,std::vector<const char*> enableLayers, std::vector<const char*> enableExtensions,const VkPhysicalDeviceFeatures& enableFeatues);
	void DestroyDevice(VkDevice device);
	VkQueue GetQueue(VkDevice device, uint32_t familyIndex,uint32_t queueIndex);

	//surface
	VkSurfaceKHR CreateWin32Surface(VkInstance instance, GLFWwindow* window);
	void DestroySurface(VkInstance instance, VkSurfaceKHR surface);
	std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);


	//resource
	
	//swapchain
	VkSwapchainKHR CreateSwapchain(VkDevice device, VkSurfaceKHR surface, VkFormat format,VkColorSpaceKHR colorSpace, VkExtent2D extent,uint32_t numLayers,uint32_t numMips, uint32_t imageCount, VkImageUsageFlags imageUsage, VkSharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices,VkPresentModeKHR presentMode);
	void DestroySwapchain(VkDevice device, VkSwapchainKHR swapchain);
	std::vector<VkImage> GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);

	//image
	VkImage CreateImage(VkDevice device, VkImageCreateFlags       flags,
		VkImageType              imageType,
		VkFormat                 format,
		VkExtent3D               extent,
		uint32_t                 mipLevels,
		uint32_t                 arrayLayers,
		VkSampleCountFlagBits    samples,
		VkImageTiling            tiling,
		VkImageUsageFlags        usage,
		VkSharingMode            sharingMode,
		std::vector<uint32_t> queueFamilyIndices = {},
		VkImageLayout            initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

	//image view
	VkImageView CreateImageView(VkDevice device, VkImageViewCreateFlags     flags,
	VkImage                    image,
	VkImageViewType            viewType,
	VkFormat                   format,
	VkComponentMapping         components,
	VkImageSubresourceRange    subresourceRange);

	//buffer
	VkBuffer CreateBuffer(VkDevice device ,   VkBufferCreateFlags    flags,
	VkDeviceSize           size,
	VkBufferUsageFlags     usage,
	VkSharingMode          sharingMode,
	std::vector<uint32_t> queueFamilyIndices);

	//buffer view   
	VkBufferView CreateBufferView(VkDevice device, VkBufferViewCreateFlags flags, VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range);

	//render pass
	VkRenderPass CreateRenderPass(VkDevice device, VkRenderPassCreateFlags           flags,
	const std::vector<VkAttachmentDescription>& attachments,
	const std::vector<VkSubpassDescription>& subpasses,
	const std::vector<VkSubpassDependency> dependencies);


	//framebuffer
	VkFramebuffer CreateFrameBuffer(VkDevice device , VkFramebufferCreateFlags    flags,
	VkRenderPass                renderPass,
	std::vector<VkImageView> attachments,
	uint32_t                    width,
	uint32_t                    height,
	uint32_t                    layers);

	
	
	//descriptor set layout
	VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, VkDescriptorSetLayoutCreateFlags flags,const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	
	//pipeline layout
	VkPipelineLayout CreatePipelineLayout(VkDevice device, VkPipelineLayoutCreateFlags     flags,
	const std::vector<VkDescriptorSetLayout> pSetLayouts,
	const std::vector<VkPushConstantRange> pushConstantRanges);
	
	//graphics pipeline
	VkPipeline CreateGraphicsPipeline(VkDevice device, VkPipelineCreateFlags                            flags,
	const std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
	const VkPipelineVertexInputStateCreateInfo* vertexInputState,
	const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState,
	const VkPipelineTessellationStateCreateInfo* tessellationState,
	const VkPipelineViewportStateCreateInfo* viewportState,
	const VkPipelineRasterizationStateCreateInfo* rasterizationState,
	const VkPipelineMultisampleStateCreateInfo* multisampleState,
	const VkPipelineDepthStencilStateCreateInfo* depthStencilState,
	const VkPipelineColorBlendStateCreateInfo* colorBlendState,
	const VkPipelineDynamicStateCreateInfo* dynamicState,
	VkPipelineLayout                                 layout,
	VkRenderPass                                     renderPass,
	uint32_t                                         subpass,
	VkPipeline                                       basePipelineHandle,
	int32_t                                          basePipelineIndex);



	//utils
	int32_t GetPhysicalDeviceSurportGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice);

};