#pragma once
#include "vulkan/vulkan.h"
#include "../Common/Log.h"
#include <GLFW/glfw3.h>
#include <vector>
namespace VulkanAPI {

	void Initialize();
	//platform
	std::vector<const char*> GetInstanceNeedWinGLFWExtensionNames();
	GLFWwindow* CreateWin32Window(int width, int height, const char* windowName);
	void DestroyGLFWWin32Window(GLFWwindow* window);

	//instance
	std::vector<VkLayerProperties> EnumerateLayerProperties();
	std::vector<VkExtensionProperties> EnumerateExtensionProperties(const char* layerName);
	VkInstance CreateInstance(std::vector<const char*> enableLayers,std::vector<const char*> enableExtensions);
	void DestroyInstance(VkInstance instance);
	void* InstanceFuncLoader(VkInstance instance, const char* funcName);
	VkDebugUtilsMessengerEXT CreateDebugInfoMessager(VkInstance instance);
	void DestroyDebugInfoMessager(VkInstance instance, VkDebugUtilsMessengerEXT debugMessager);

	std::vector<VkPhysicalDevice> EnumeratePhysicalDevice(VkInstance instance);
	std::vector<VkPhysicalDeviceProperties> EnumeratePhysicalDeviceProperties(VkInstance instance);
	std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice);
	bool GetQueueFamilySurfaceSupport(VkPhysicalDevice  physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface);
	
	//physical device
	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
	VkPhysicalDeviceProperties2 GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,void* pNext = nullptr);

	void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2& physicalDeviceFeatures2);
	VkPhysicalDeviceMemoryProperties GetMemoryProperties(VkPhysicalDevice physicalDevice);
	VkFormatProperties GetFormatPropetirs(VkPhysicalDevice physicalDevice, VkFormat format);
	std::vector<VkExtensionProperties> EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* layerName);
	std::vector<VkLayerProperties> EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice);
	VkImageFormatProperties GetImageFormatProperties(VkPhysicalDevice  physicalDevice,VkFormat  format,VkImageType  type,VkImageTiling  tiling,VkImageUsageFlags  usage,VkImageCreateFlags  flags);


	//device
	VkDevice CreateDevice(VkPhysicalDevice physicalDevice, const std::vector<std::pair<uint32_t,std::vector<float>>>& wantQueueFamilyAndQueuePriorities,std::vector<const char*> enableLayers, std::vector<const char*> enableExtensions,const VkPhysicalDeviceFeatures* enableFeatues,const void* extendInfoPointer = nullptr);
	void DestroyDevice(VkDevice device);
	VkQueue GetQueue(VkDevice device, uint32_t familyIndex,uint32_t queueIndex);
	void DeviceWaitIdle(VkDevice device);
	void* DeviceFuncLoader(VkDevice device, const char* funcName);
	//加载扩展的API,让该文件封装的扩展API可以使用
	void LoadExtensionAPIs(VkDevice device);

	//surface
	VkSurfaceKHR CreateWin32Surface(VkInstance instance, GLFWwindow* window);
	void DestroySurface(VkInstance instance, VkSurfaceKHR surface);
	std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	std::vector<VkPresentModeKHR> GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	//resource
	
	//swapchain
	VkSwapchainKHR CreateSwapchain(VkDevice device, VkSurfaceKHR surface, VkFormat format,VkColorSpaceKHR colorSpace, VkExtent2D extent,uint32_t numLayers,uint32_t numMips, uint32_t imageCount, VkImageUsageFlags imageUsage, VkSharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices,VkPresentModeKHR presentMode);
	void DestroySwapchain(VkDevice device, VkSwapchainKHR swapchain);
	std::vector<VkImage> GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);
	uint32_t GetNextValidSwapchainImageIndex(VkDevice device, VkSwapchainKHR swapchain ,VkSemaphore  semaphore,VkFence  fence);

	
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
	void DestroyImage(VkDevice device, VkImage image);

	VkMemoryRequirements GetImageMemoryRequirments(VkDevice device, VkImage image);
	VkSubresourceLayout GetImageSubresourceLayout(VkDevice device,VkImage image,const VkImageSubresource& subresource);


	//image view
	VkImageView CreateImageView(VkDevice device, VkImageViewCreateFlags     flags,
	VkImage                    image,
	VkImageUsageFlags          usage,
	VkImageViewType            viewType,
	VkFormat                   format,
	VkComponentMapping         components,
	VkImageSubresourceRange    subresourceRange);
	void DestroyImageView(VkDevice device, VkImageView imageView);


	//buffer
	VkBuffer CreateBuffer(VkDevice device ,   VkBufferCreateFlags    flags,
	VkDeviceSize           size,
	VkBufferUsageFlags     usage,
	VkSharingMode          sharingMode,
	std::vector<uint32_t> queueFamilyIndices);
	void DestroyBuffer(VkDevice device, VkBuffer buffer);
	VkMemoryRequirements GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer);

	VkDeviceAddress GetBufferDeviceAddressKHR(VkDevice device,VkBuffer buffer);

	//buffer view   
	VkBufferView CreateBufferView(VkDevice device, VkBufferViewCreateFlags flags, VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range);
	void DestroyBufferView(VkDevice device, VkBufferView bufferView);


	VkAccelerationStructureKHR CreateAccelerationStructureKHR(VkDevice device, 
														VkAccelerationStructureCreateFlagsKHR    createFlags,
														VkBuffer                                 buffer,
														VkDeviceSize                             offset,
														VkDeviceSize                             size,
														VkAccelerationStructureTypeKHR           type);


	void DestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructureKHR);

	VkAccelerationStructureBuildSizesInfoKHR GetAccelerationStructureBuildSizesKHR(VkDevice device, const VkAccelerationStructureBuildGeometryInfoKHR& accelerationStructureBuildGeometryInfoKHR);
	//void BuildBottomAccelerationStructureKHR(VkDevice device,const std::vector<VkAccelerationStructureGeometryKHR>& geomsInfos, const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& buildRanges,VkAccelerationStructureKHR accelerationStructureKHR);

	void GetRayTracingShaderGroupHandlesKHR(VkDevice  device,VkPipeline  pipeline,uint32_t  firstGroup,uint32_t  groupCount,std::vector<char>& outData);


	//memory 
	VkDeviceMemory AllocateMemory(VkDevice device,
		VkDeviceSize       allocationSize,
		uint32_t           memoryTypeIndex, VkMemoryAllocateFlags allocationFlags);
	void ReleaseMemory(VkDevice device, VkDeviceMemory deviceMemory);

	void* MapMemory(VkDevice device, VkDeviceMemory deviceMemory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags mapFlags);
	void UnmapMemory(VkDevice device, VkDeviceMemory deviceMemory);
	void BindMemoryToImage(VkDevice device, VkDeviceMemory deviceMemory, VkImage image, VkDeviceSize offset);
	void BindMemoryToBuffer(VkDevice device, VkDeviceMemory deviceMemory, VkBuffer buffer, VkDeviceSize offset);



	//render pass
	VkRenderPass CreateRenderPass(VkDevice device, VkRenderPassCreateFlags           flags,
	const std::vector<VkAttachmentDescription>& attachments,
	const std::vector<VkSubpassDescription>& subpasses,
	const std::vector<VkSubpassDependency> dependencies);
	void DestroyRenderPass(VkDevice device, VkRenderPass renderPass);

	//framebuffer
	VkFramebuffer CreateFrameBuffer(VkDevice device , VkFramebufferCreateFlags    flags,
	VkRenderPass                renderPass,
	std::vector<VkImageView> attachments,
	uint32_t                    width,
	uint32_t                    height,
	uint32_t                    layers);
	void DestroyFrameBuffer(VkDevice device, VkFramebuffer frameBuffer);


	
	
	//descriptor set layout
	VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, VkDescriptorSetLayoutCreateFlags flags,const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	void DestroyDesctriptorSetLayout(VkDevice device, VkDescriptorSetLayout desctriptorSetLayout);

	VkSampler CreateSampler(VkDevice device,
				VkSamplerCreateFlags    flags,
				VkFilter                magFilter,
				VkFilter                minFilter,
				VkSamplerMipmapMode     mipmapMode,
				VkSamplerAddressMode    addressModeU,
				VkSamplerAddressMode    addressModeV,
				VkSamplerAddressMode    addressModeW,
				float                   mipLodBias,
				VkBool32                anisotropyEnable,
				float                   maxAnisotropy,
				VkBool32                compareEnable,
				VkCompareOp             compareOp,
				float                   minLod,
				float                   maxLod,
				VkBorderColor           borderColor,
				VkBool32                unnormalizedCoordinates);
	VkSampler CreateDefaultSampler(VkDevice device,float maxLod);
	void DestroySampler(VkDevice device,VkSampler sampler);

	//pipeline layout
	VkPipelineLayout CreatePipelineLayout(VkDevice device, VkPipelineLayoutCreateFlags     flags,
			const std::vector<VkDescriptorSetLayout> pSetLayouts,
			const std::vector<VkPushConstantRange> pushConstantRanges);
	void DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout);

	//shader module
	VkShaderModule CreateShaderModule(VkDevice device,VkShaderModuleCreateFlags flags,const std::vector<uint32_t>& spirv_code);
	void DestroyShaderModule(VkDevice device, VkShaderModule shaderModule);


	//graphics pipeline
	VkPipeline CreateGraphicsPipeline(VkDevice device, VkPipelineCreateFlags                            flags,
	const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
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
	uint32_t                                         subpass);
	VkPipeline CreateComputePipeline(VkDevice device, 
	VkPipelineCreateFlags              flags,
	VkPipelineShaderStageCreateInfo    stage,
	VkPipelineLayout                   layout);

	VkPipeline CreateRayTracingPipeline(VkDevice device,const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,uint32_t maxPipelineRayRecursionDepth,VkPipelineLayout pipelineLayout);



	void DestroyPipeline(VkDevice device, VkPipeline pipeline);

	//descriptor pool
	VkDescriptorPool CreateDescriptorPool(VkDevice device,
		VkDescriptorPoolCreateFlags    flags,
		uint32_t                       maxSets,
		const std::vector<VkDescriptorPoolSize> poolSizes);
	void DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool);

 
	//descriptor set
	VkDescriptorSet AllocateDescriptorSet(VkDevice device,
		VkDescriptorPool                descriptorPool,
		const std::vector<VkDescriptorSetLayout> setLayouts);
	void ReleaseDescriptorSets(VkDevice device,VkDescriptorPool descriptorPool,const std::vector<VkDescriptorSet>& descriptorSets);

	//update descriptor set
	void UpdateDescriptorSetBindingResources(VkDevice device,
											 VkDescriptorSet                  dstSet,
											 uint32_t                         dstBinding,
											 uint32_t                         dstArrayElement,
											 uint32_t                         descriptorCount,
											 VkDescriptorType                 descriptorType,
											 const std::vector<VkDescriptorImageInfo> imageInfos,
											 const std::vector<VkDescriptorBufferInfo> bufferInfos,
											 const std::vector<VkBufferView> texelBufferViews,
											void* pNext = nullptr);
	
	//query pool
	VkQueryPool CreateQueryPool(VkDevice device, VkQueryPoolCreateFlags  flags,VkQueryType     queryType, uint32_t    queryCount, VkQueryPipelineStatisticFlags    pipelineStatistics);
	void DestroyQueryPool(VkDevice device, VkQueryPool pool);
	void ResetQueryPool(VkDevice device, VkQueryPool pool, uint32_t firstQuery, uint32_t numQuery);
	std::vector<uint64_t> GetQueryResult(VkDevice device, VkQueryPool pool, uint32_t firstQuery, uint32_t numQuery, VkQueryResultFlags resultFlags);

	//command pool
	VkCommandPool CreateCommandPool(VkDevice device, VkCommandPoolCreateFlags flags,uint32_t queueFamilyIndex);
	void DesctroyCommandPool(VkDevice device, VkCommandPool commandPool);
	
	//command buffer
	VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel    level);
	void ReleaseCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
	
	std::vector<VkCommandBuffer> AllocateCommandBuffers(VkDevice device, VkCommandPool commandPool,VkCommandBufferLevel  level, uint32_t commandBufferCount);
	void ReleaseCommandBuffers(VkDevice device, VkCommandPool commandPool, const std::vector<VkCommandBuffer>& commandBuffers);;

	//fence
	VkFence CreateFence(VkDevice device, VkFenceCreateFlags flags);
	void DestroyFence(VkDevice device, VkFence fence);
	VkResult WaitFence(VkDevice device, const std::vector<VkFence>& fences, bool waitAll);
	void ResetFences(VkDevice device, const std::vector<VkFence>& fences);
	VkResult GetFenceStatus(VkDevice device, VkFence fence);

	//semaphore
	VkSemaphore Create_Semaphore_(VkDevice device, VkSemaphoreCreateFlags flags);
	void DestroySemaphore(VkDevice device, VkSemaphore semaphore);
	void WaitTimelineSemaphores(VkDevice device, VkSemaphoreWaitFlags    flags,
						const std::vector<VkSemaphore>& semaphores,
						const std::vector<uint64_t>& semaphoreValues);
	void SignalSemaphores(VkDevice device,VkSemaphore semaphore, uint64_t value = 0/*只对timeline semaphore有用*/);

	//event
	VkEvent CreateEvent(VkDevice device, VkEventCreateFlags flags);
	void DestroyEvent(VkDevice device, VkEvent c_event);


	//---------------------------------------------------------------------------------
	// commands
	void CommandBufferReset(VkCommandBuffer commandBuffer);
	void BeginRecord(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags  flags);
	void EndRecord(VkCommandBuffer commandBuffer);
	void CmdBeginRenderPass(VkCommandBuffer commandBuffer, 
							VkRenderPass           renderPass,
							VkFramebuffer          framebuffer,
							VkRect2D               renderArea,
							const std::vector<VkClearValue>& cleatValues,
							VkSubpassContents  subpassContents);
	void CmdEndRenderPass(VkCommandBuffer commandBuffer);
	void CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents  subpassContents);

	void CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t queryIndex, VkQueryControlFlags flags);
	void CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t queryIndex);
	void CmdResetQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQueryIndex, uint32_t numQuery);


	//动态绘制,可以不使用begin pass
	void CmdBeginRendering(VkCommandBuffer commandBuffer, 
						VkRenderingFlags                    flags,
						VkRect2D                            renderArea,
						uint32_t                            layerCount,
						uint32_t                            viewMask,
						const std::vector<VkRenderingAttachmentInfo>& colorAttachments,
						const VkRenderingAttachmentInfo* depthAttachment,
						const VkRenderingAttachmentInfo* stencilAttachment);
	void CmdEndRendering(VkCommandBuffer commandBuffer);

	void CmdBindVertexBuffers(VkCommandBuffer  commandBuffer,
							uint32_t  firstBinding,
							const std::vector<VkBuffer>& buffers,
							const std::vector<VkDeviceSize>& offsets);
	void CmdBindIndexBuffer(VkCommandBuffer  commandBuffer,
							VkBuffer      buffer,
							VkDeviceSize  offset,
							VkIndexType   indexType);

	void CmdBindPipeline(VkCommandBuffer  commandBuffer,
						VkPipelineBindPoint  pipelineBindPoint,
						VkPipeline  pipeline);
	void CmdBindPipelineLayout(VkCommandBuffer  commandBuffer);
	void CmdBindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
						uint32_t firstSet, const std::vector<VkDescriptorSet> descriptorSets, const std::vector<uint32_t> dynamicOffsets);
	//dynamic state
	void CmdDynamicSetViewPorts(VkCommandBuffer commandBuffer, uint32_t firstViewport, const std::vector<VkViewport>& viewports);
	void CmdClearAttachments(VkCommandBuffer commandBuffer, const std::vector<VkClearAttachment>& clearAttachments, const std::vector<VkClearRect>& clearRegions);
	void CmdClearDepthImage(VkCommandBuffer commandBuffer, VkImage depthImage, VkImageLayout imageLayout, const VkClearDepthStencilValue& clearValue, const std::vector<VkImageSubresourceRange>& clearRegions);
	void CmdDrawVertex(VkCommandBuffer  commandBuffer,
						uint32_t  vertexCount,
						uint32_t  instanceCount,
						uint32_t  firstVertex,
						uint32_t  firstInstance);

	void CmdDrawMeshTasksEXT(VkCommandBuffer  commandBuffer,
		uint32_t                                    groupCountX,
		uint32_t                                    groupCountY,
		uint32_t                                    groupCountZ);
	void CmdDrawIndex(VkCommandBuffer   commandBuffer,
					uint32_t  indexCount,
					uint32_t  instanceCount,
					uint32_t  firstIndex,
					int32_t   vertexOffset,
					uint32_t  firstInstance);

	void CmdClearColorImage(VkCommandBuffer  commandBuffer,
					VkImage                                     image,
					VkImageLayout                               imageLayout,
					const std::vector<VkClearColorValue>& claerColors,
					const std::vector<VkImageSubresourceRange>& clearRanges);

	void CmdCopyImageToImage(VkCommandBuffer commandBuffer,
					VkImage                                     srcImage,
					VkImageLayout                               srcImageLayout,
					VkImage                                     dstImage,
					VkImageLayout                               dstImageLayout,
					const std::vector<VkImageCopy>& copyRegions);

	void CmdBlitImageToImage(VkCommandBuffer commandBuffer,
					VkImage    srcImage,
					VkImageLayout srcImageLayout,
					VkImage  dstImage,
					VkImageLayout dstImageLayout,
					const std::vector<VkImageBlit> blitRegions,
					VkFilter  filter = VK_FILTER_LINEAR);

	void CmdMemoryBarrier(VkCommandBuffer                             commandBuffer,
						 VkPipelineStageFlags                        srcStageMask,
						 VkPipelineStageFlags                        dstStageMask,
						 VkDependencyFlags                           dependencyFlags,
						 const std::vector<VkMemoryBarrier>& memoryBarriers,
						 const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers,
						 const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers);


	//compute cmd
	void CmdDispatch(VkCommandBuffer  commandBuffer,uint32_t  groupCountX,uint32_t  groupCountY,uint32_t  groupCountZ);


	//ray tracing cmd
	void CmdBuildBottomAccelerationStructureKHR(VkCommandBuffer commandBuffer, const std::vector<VkAccelerationStructureGeometryKHR>& geomsInfos, const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& buildRanges, VkAccelerationStructureKHR accelerationStructureKHR, const VkDeviceOrHostAddressKHR& scratchData);

	void CmdTraceRaysKHR(VkCommandBuffer                             commandBuffer,
		const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
		uint32_t                                    width,
		uint32_t                                    height,
		uint32_t                                    depth);



	
	//-------------------------------------------------------------------------------------
	//submit and present
	void SubmitCommands(VkQueue  queue,
						const std::vector<VkSemaphore>& waitSemaphores,
						const std::vector<VkPipelineStageFlags>& waitDstStageMask,
						const std::vector<VkCommandBuffer>& commandBuffers,
						const std::vector<VkSemaphore>& signalSemaphores,
						VkFence allCommandFinishedFence);
	void SubmitCommands(VkQueue queue, const std::vector<VkSubmitInfo>& submitInfos, VkFence allCommandFinishedFence);

	void Present(VkQueue queue,
				const std::vector<VkSemaphore>& waitSemaphores,
				const std::vector<VkSwapchainKHR>& swapchains,
				const std::vector<uint32_t>& swapchainImageIndices,
				std::vector<VkResult>& outResults);
	void WaitQueueIdle(VkQueue queue);

	//
	
	
	
	//utils



};