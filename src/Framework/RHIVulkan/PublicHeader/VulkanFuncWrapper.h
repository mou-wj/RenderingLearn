#pragma once
#include "vulkan/vulkan.h"
#ifdef _WIN32
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#include <QuartzCore/CAMetalLayer.h>
#elif defined(__linux__)
#include <vulkan/vulkan_xlib.h>
#include <X11/Xlib.h>
#endif
namespace RHIVulkan {

    // ---------------------------
    // Instance
    // ---------------------------
    bool CreateInstance(const VkInstanceCreateInfo* pCreateInfo, VkInstance* pInstance);
    bool DestroyInstance(VkInstance instance);

    bool CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
        VkSurfaceKHR* pSurface);
    bool DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface);

    bool EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
    bool GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyCount, VkQueueFamilyProperties* pQueueFamilies);

    bool GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
    bool GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pCapabilities);
    bool GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
    bool GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);

    // ---------------------------
    // Device
    // ---------------------------
    bool CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, VkDevice* pDevice);
    bool DestroyDevice(VkDevice device);
    bool GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);

    // ---------------------------
    // Command Pool / Command Buffer
    // ---------------------------
    bool CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, VkCommandPool* pCommandPool);
    bool DestroyCommandPool(VkDevice device, VkCommandPool commandPool);
    bool AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
    bool FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
    bool BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
    bool EndCommandBuffer(VkCommandBuffer commandBuffer);

    bool ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags = 0);

    //cmd
    void CmdCopyBufferToImage(
        VkCommandBuffer                             commandBuffer,
        VkBuffer                                    srcBuffer,
        VkImage                                     dstImage,
        VkImageLayout                               dstImageLayout,
        uint32_t                                    regionCount,
        const VkBufferImageCopy* pRegions);


    // ---------------------------
    // Synchronization
    // ---------------------------
    bool CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, VkSemaphore* pSemaphore);
    bool DestroySemaphore(VkDevice device, VkSemaphore semaphore);

    bool CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, VkFence* pFence);
    bool DestroyFence(VkDevice device, VkFence fence);
    bool WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
    bool ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);

    // ---------------------------
    // Buffer / Image / Memory
    // ---------------------------
    bool CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, VkBuffer* pBuffer);
    bool DestroyBuffer(VkDevice device, VkBuffer buffer);

    bool CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, VkImage* pImage);
    bool DestroyImage(VkDevice device, VkImage image);

    bool CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, VkImageView* pImageView);

    void DestroyImageView(VkDevice device, VkImageView imageView);

    bool GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
    bool GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);

    bool AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory);
    bool FreeMemory(VkDevice device, VkDeviceMemory memory);
    bool BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset);
    bool BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize offset);

    // ---------------------------
    // Swapchain
    // ---------------------------
    bool CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain);
    bool DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain);
    bool AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
    bool QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

    // ---------------------------
    // Pipeline / RenderPass / Framebuffer
    // ---------------------------
    bool CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, VkRenderPass* pRenderPass);
    bool DestroyRenderPass(VkDevice device, VkRenderPass renderPass);

    bool CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, VkFramebuffer* pFramebuffer);
    bool DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer);

    bool CreateGraphicsPipeline(VkDevice device, const VkGraphicsPipelineCreateInfo* pCreateInfo, VkPipeline* pPipeline);
    bool DestroyPipeline(VkDevice device, VkPipeline pipeline);

    bool CreateComputePipeline(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo, VkPipeline* pPipeline);

    // ---------------------------
    // Descriptor Sets
    // ---------------------------
    bool CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, VkDescriptorPool* pDescriptorPool);
    bool DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool);
    bool AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
    bool FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);

    // ---------------------------
    // Utility
    // ---------------------------
    bool QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    bool DeviceWaitIdle(VkDevice device);
    bool QueueWaitIdle(VkQueue queue);




}