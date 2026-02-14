#include "VulkanFuncWrapper.h"
#include "Log.h"

namespace RHIVulkan {
    // ÄÚ²¿ºê£º¼ì²é VkResult
    // ---------------------------
#define CHECK_RESULT(res, msg) \
        do { \
            if ((res) != VK_SUCCESS) { LOG_ERROR("%s failed, VkResult=%d", msg, (int)res); return false; } \
        } while(0)

    // ---------------------------
    // Instance
    // ---------------------------
    bool CreateInstance(const VkInstanceCreateInfo* pCreateInfo, VkInstance* pInstance)
    {
#ifdef DEBUG_INFO
        if (!pCreateInfo || !pInstance) { LOG_ERROR("CreateInstance: null pointer"); return false; }
#endif
        VkResult res = vkCreateInstance(pCreateInfo, nullptr, pInstance);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateInstance");
#endif
        LOG_INFO("CreateInstance success");
        return res == VK_SUCCESS;
    }

    bool DestroyInstance(VkInstance instance)
    {
        if (!instance) return false;
        vkDestroyInstance(instance, nullptr);
        LOG_INFO("DestroyInstance called");
        return true;
    }

    bool CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
        VkSurfaceKHR* pSurface)
    {
#ifdef DEBUG_INFO
        if (!instance || !pCreateInfo || !pSurface) { LOG_ERROR("CreateSurfaceKHR: null pointer"); return false; }
#endif
        VkResult res = vkCreateWin32SurfaceKHR(instance, pCreateInfo, nullptr, pSurface);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateSurfaceKHR");
#endif
        LOG_INFO("CreateSurfaceKHR success");
        return true;
    }

    bool DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface)
    {
        if (!instance || !surface) return false;
        vkDestroySurfaceKHR(instance, surface, nullptr);
        LOG_INFO("DestroySurfaceKHR called");
        return true;
    }

    bool EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
    {
#ifdef DEBUG_INFO
        if (!instance || !pPhysicalDeviceCount) { LOG_ERROR("EnumeratePhysicalDevices: null pointer"); return false; }
#endif
        VkResult res = vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "EnumeratePhysicalDevices");
#endif
        LOG_INFO("EnumeratePhysicalDevices count=%d", *pPhysicalDeviceCount);
        return true;
    }

    bool GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyCount, VkQueueFamilyProperties* pQueueFamilies)
    {
#ifdef DEBUG_INFO
        if (!physicalDevice || !pQueueFamilyCount) { LOG_ERROR("GetPhysicalDeviceQueueFamilyProperties: null pointer"); return false; }
#endif
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyCount, pQueueFamilies);
        LOG_INFO("GetPhysicalDeviceQueueFamilyProperties count=%d", *pQueueFamilyCount);
        return true;
    }

    bool GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
    {
#ifdef DEBUG_INFO
        if (!physicalDevice || !pSupported) { LOG_ERROR("GetPhysicalDeviceSurfaceSupportKHR: null pointer"); return false; }
#endif
        VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "GetPhysicalDeviceSurfaceSupportKHR");
#endif
        return true;
    }

    bool GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pCapabilities)
    {
#ifdef DEBUG_INFO
        if (!physicalDevice || !pCapabilities) { LOG_ERROR("GetPhysicalDeviceSurfaceCapabilitiesKHR: null pointer"); return false; }
#endif
        VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pCapabilities);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "GetPhysicalDeviceSurfaceCapabilitiesKHR");
#endif
        return true;
    }

    bool GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
    {
#ifdef DEBUG_INFO
        if (!physicalDevice || !pSurfaceFormatCount) { LOG_ERROR("GetPhysicalDeviceSurfaceFormatsKHR: null pointer"); return false; }
#endif
        VkResult res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "GetPhysicalDeviceSurfaceFormatsKHR");
#endif
        return true;
    }

    bool GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
    {
#ifdef DEBUG_INFO
        if (!physicalDevice || !pPresentModeCount) { LOG_ERROR("GetPhysicalDeviceSurfacePresentModesKHR: null pointer"); return false; }
#endif
        VkResult res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "GetPhysicalDeviceSurfacePresentModesKHR");
#endif
        return true;
    }

    // ---------------------------
    // Device / Queue
    // ---------------------------
    bool CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, VkDevice* pDevice)
    {
#ifdef DEBUG_INFO
        if (!physicalDevice || !pCreateInfo || !pDevice) { LOG_ERROR("CreateDevice: null pointer"); return false; }
#endif
        VkResult res = vkCreateDevice(physicalDevice, pCreateInfo, nullptr, pDevice);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateDevice");
#endif
        LOG_INFO("CreateDevice success");
        return true;
    }

    bool DestroyDevice(VkDevice device)
    {
        if (!device) return false;
        vkDestroyDevice(device, nullptr);
        LOG_INFO("DestroyDevice called");
        return true;
    }

    bool GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
    {
#ifdef DEBUG_INFO
        if (!device || !pQueue) { LOG_ERROR("GetDeviceQueue: null pointer"); return false; }
#endif
        vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        LOG_INFO("GetDeviceQueue called queueFamily=%d queueIndex=%d", queueFamilyIndex, queueIndex);
        return true;
    }

    // ---------------------------
    // Command Pool / Command Buffer
    // ---------------------------
    bool CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, VkCommandPool* pCommandPool)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pCommandPool) { LOG_ERROR("CreateCommandPool: null pointer"); return false; }
#endif
        VkResult res = vkCreateCommandPool(device, pCreateInfo, nullptr, pCommandPool);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateCommandPool");
#endif
        LOG_INFO("CreateCommandPool success");
        return true;
    }

    bool DestroyCommandPool(VkDevice device, VkCommandPool commandPool)
    {
        if (!device || !commandPool) return false;
        vkDestroyCommandPool(device, commandPool, nullptr);
        LOG_INFO("DestroyCommandPool called");
        return true;
    }

    bool AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
    {
#ifdef DEBUG_INFO
        if (!device || !pAllocateInfo || !pCommandBuffers) { LOG_ERROR("AllocateCommandBuffers: null pointer"); return false; }
#endif
        VkResult res = vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "AllocateCommandBuffers");
#endif
        LOG_INFO("AllocateCommandBuffers count=%d", pAllocateInfo->commandBufferCount);
        return true;
    }

    bool FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
    {
#ifdef DEBUG_INFO
        if (!device || !commandPool || !pCommandBuffers) { LOG_ERROR("FreeCommandBuffers: null pointer"); return false; }
#endif
        vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        LOG_INFO("FreeCommandBuffers count=%d", commandBufferCount);
        return true;
    }

    bool BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
    {
#ifdef DEBUG_INFO
        if (!commandBuffer || !pBeginInfo) { LOG_ERROR("BeginCommandBuffer: null pointer"); return false; }
#endif
        VkResult res = vkBeginCommandBuffer(commandBuffer, pBeginInfo);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "BeginCommandBuffer");
#endif
        return true;
    }

    bool EndCommandBuffer(VkCommandBuffer commandBuffer)
    {
#ifdef DEBUG_INFO
        if (!commandBuffer) { LOG_ERROR("EndCommandBuffer: null pointer"); return false; }
#endif
        VkResult res = vkEndCommandBuffer(commandBuffer);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "EndCommandBuffer");
#endif
        return true;
    }

    bool ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
    {
#ifdef DEBUG_INFO
        if (!commandBuffer) { LOG_ERROR("ResetCommandBuffer: null pointer"); return false; }
#endif
        VkResult res = vkResetCommandBuffer(commandBuffer, flags);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "ResetCommandBuffer");
#endif
        return true;
    }

    void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
    {
#ifdef DEBUG_INFO
		if (!commandBuffer || !srcBuffer || !dstImage || !pRegions) { LOG_ERROR("CmdCopyBufferToImage: null pointer"); return; }
#endif
		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }

    // ---------------------------
    // Semaphore / Fence
    // ---------------------------
    bool CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, VkSemaphore* pSemaphore)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pSemaphore) { LOG_ERROR("CreateSemaphore: null pointer"); return false; }
#endif
        VkResult res = vkCreateSemaphore(device, pCreateInfo, nullptr, pSemaphore);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateSemaphore");
#endif
        LOG_INFO("CreateSemaphore success");
        return true;
    }

    bool DestroySemaphore(VkDevice device, VkSemaphore semaphore)
    {
#ifdef DEBUG_INFO
        if (!device || !semaphore) return false;
#endif
        vkDestroySemaphore(device, semaphore, nullptr);
        LOG_INFO("DestroySemaphore called");
        return true;
    }

    bool CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, VkFence* pFence)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pFence) { LOG_ERROR("CreateFence: null pointer"); return false; }
#endif
        VkResult res = vkCreateFence(device, pCreateInfo, nullptr, pFence);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateFence");
#endif
        LOG_INFO("CreateFence success");
        return true;
    }

    bool DestroyFence(VkDevice device, VkFence fence)
    {
        if (!device || !fence) return false;
        vkDestroyFence(device, fence, nullptr);
        LOG_INFO("DestroyFence called");
        return true;
    }

    bool WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
    {
#ifdef DEBUG_INFO
        if (!device || !pFences) { LOG_ERROR("WaitForFences: null pointer"); return false; }
#endif
        VkResult res = vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "WaitForFences");
#endif
        return true;
    }

    bool ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
    {
#ifdef DEBUG_INFO
        if (!device || !pFences) { LOG_ERROR("ResetFences: null pointer"); return false; }
#endif
        VkResult res = vkResetFences(device, fenceCount, pFences);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "ResetFences");
#endif
        return true;
    }

    // ---------------------------
// Buffer
// ---------------------------

    bool CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, VkBuffer* pBuffer)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pBuffer)
        {
            LOG_ERROR("CreateBuffer failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateBuffer(device, pCreateInfo, nullptr, pBuffer);

#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateBuffer failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool DestroyBuffer(VkDevice device, VkBuffer buffer)
    {
#ifdef DEBUG_INFO
        if (!device || buffer == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyBuffer skipped: invalid handle");
            return false;
        }
#endif
        vkDestroyBuffer(device, buffer, nullptr);
        return true;
    }

    // ---------------------------
    // Image
    // ---------------------------

    bool CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, VkImage* pImage)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pImage)
        {
            LOG_ERROR("CreateImage failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateImage(device, pCreateInfo, nullptr, pImage);

#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateImage failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool DestroyImage(VkDevice device, VkImage image)
    {
#ifdef DEBUG_INFO
        if (!device || image == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyImage skipped: invalid handle");
            return false;
        }
#endif
        vkDestroyImage(device, image, nullptr);
        return true;
    }

    // ---------------------------
    // ImageView
    // ---------------------------

    bool CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, VkImageView* pImageView)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pImageView)
        {
            LOG_ERROR("CreateImageView failed: invalid parameters");
            return false;
        }
        if (pCreateInfo->image == VK_NULL_HANDLE)
        {
            LOG_ERROR("CreateImageView failed: pCreateInfo->image is null");
            return false;
        }
#endif
        VkResult result = vkCreateImageView(device, pCreateInfo, nullptr, pImageView);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateImageView failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    void DestroyImageView(VkDevice device, VkImageView imageView)
    {
#ifdef DEBUG_INFO
        if (!device || imageView == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyImageView skipped: invalid handle");
            return;
        }
#endif
        vkDestroyImageView(device, imageView, nullptr);
    }

    // ---------------------------
    // Memory
    // ---------------------------

    bool GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
    {
#ifdef DEBUG_INFO
        if (!device || buffer == VK_NULL_HANDLE || !pMemoryRequirements)
        {
            LOG_ERROR("GetBufferMemoryRequirements failed: invalid parameters");
            return false;
        }
#endif
        vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        return true;
    }

    bool GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
    {
#ifdef DEBUG_INFO
        if (!device || image == VK_NULL_HANDLE || !pMemoryRequirements)
        {
            LOG_ERROR("GetImageMemoryRequirements failed: invalid parameters");
            return false;
        }
#endif
        vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
        return true;
    }

    bool AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory)
    {
#ifdef DEBUG_INFO
        if (!device || !pAllocateInfo || !pMemory)
        {
            LOG_ERROR("AllocateMemory failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkAllocateMemory(device, pAllocateInfo, nullptr, pMemory);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkAllocateMemory failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool FreeMemory(VkDevice device, VkDeviceMemory memory)
    {
#ifdef DEBUG_INFO
        if (!device || memory == VK_NULL_HANDLE)
        {
            LOG_WARN("FreeMemory skipped: invalid handle");
            return false;
        }
#endif
        vkFreeMemory(device, memory, nullptr);
        return true;
    }

    bool BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset)
    {
#ifdef DEBUG_INFO
        if (!device || buffer == VK_NULL_HANDLE || memory == VK_NULL_HANDLE)
        {
            LOG_ERROR("BindBufferMemory failed: invalid handle");
            return false;
        }
#endif
        VkResult result = vkBindBufferMemory(device, buffer, memory, offset);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkBindBufferMemory failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize offset)
    {
#ifdef DEBUG_INFO
        if (!device || image == VK_NULL_HANDLE || memory == VK_NULL_HANDLE)
        {
            LOG_ERROR("BindImageMemory failed: invalid handle");
            return false;
        }
#endif
        VkResult result = vkBindImageMemory(device, image, memory, offset);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("BindImageMemory failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }





    // ---------------------------
    // Swapchain
    // ---------------------------
    bool CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pSwapchain) { LOG_ERROR("CreateSwapchainKHR: null pointer"); return false; }
#endif
        VkResult res = vkCreateSwapchainKHR(device, pCreateInfo, nullptr, pSwapchain);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "CreateSwapchainKHR");
#endif
        LOG_INFO("CreateSwapchainKHR success");
        return true;
    }

    bool DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain)
    {
        if (!device || !swapchain) return false;
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        LOG_INFO("DestroySwapchainKHR called");
        return true;
    }

    bool AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
    {
#ifdef DEBUG_INFO
        if (!device || !swapchain || !pImageIndex) { LOG_ERROR("AcquireNextImageKHR: null pointer"); return false; }
#endif
        VkResult res = vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "AcquireNextImageKHR");
#endif
        LOG_INFO("AcquireNextImageKHR index=%d", *pImageIndex);
        return true;
    }

    bool QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
    {
#ifdef DEBUG_INFO
        if (!queue || !pPresentInfo) { LOG_ERROR("QueuePresentKHR: null pointer"); return false; }
#endif
        VkResult res = vkQueuePresentKHR(queue, pPresentInfo);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "QueuePresentKHR");
#endif
        return true;
    }


    // ---------------------------
    // RenderPass
    // ---------------------------

    bool CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, VkRenderPass* pRenderPass)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pRenderPass)
        {
            LOG_ERROR("CreateRenderPass failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateRenderPass(device, pCreateInfo, nullptr, pRenderPass);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateRenderPass failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool DestroyRenderPass(VkDevice device, VkRenderPass renderPass)
    {
#ifdef DEBUG_INFO
        if (!device || renderPass == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyRenderPass skipped: invalid handle");
            return false;
        }
#endif
        vkDestroyRenderPass(device, renderPass, nullptr);
        return true;
    }

    // ---------------------------
    // Framebuffer
    // ---------------------------

    bool CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, VkFramebuffer* pFramebuffer)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pFramebuffer)
        {
            LOG_ERROR("CreateFramebuffer failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateFramebuffer(device, pCreateInfo, nullptr, pFramebuffer);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateFramebuffer failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer)
    {
#ifdef DEBUG_INFO
        if (!device || framebuffer == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyFramebuffer skipped: invalid handle");
            return false;
        }
#endif
        vkDestroyFramebuffer(device, framebuffer, nullptr);
        return true;
    }

    // ---------------------------
    // Graphics / Compute Pipeline
    // ---------------------------

    bool CreateGraphicsPipeline(VkDevice device, const VkGraphicsPipelineCreateInfo* pCreateInfo, VkPipeline* pPipeline)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pPipeline)
        {
            LOG_ERROR("CreateGraphicsPipeline failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, pCreateInfo, nullptr, pPipeline);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateGraphicsPipelines failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool DestroyPipeline(VkDevice device, VkPipeline pipeline)
    {
#ifdef DEBUG_INFO
        if (!device || pipeline == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyPipeline skipped: invalid handle");
            return false;
        }
#endif
        vkDestroyPipeline(device, pipeline, nullptr);
        return true;
    }

    bool CreateComputePipeline(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo, VkPipeline* pPipeline)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pPipeline)
        {
            LOG_ERROR("CreateComputePipeline failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, pCreateInfo, nullptr, pPipeline);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateComputePipelines failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    // ---------------------------
    // Descriptor Sets / Pools
    // ---------------------------

    bool CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, VkDescriptorPool* pDescriptorPool)
    {
#ifdef DEBUG_INFO
        if (!device || !pCreateInfo || !pDescriptorPool)
        {
            LOG_ERROR("CreateDescriptorPool failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkCreateDescriptorPool(device, pCreateInfo, nullptr, pDescriptorPool);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkCreateDescriptorPool failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool)
    {
#ifdef DEBUG_INFO
        if (!device || descriptorPool == VK_NULL_HANDLE)
        {
            LOG_WARN("DestroyDescriptorPool skipped: invalid handle");
            return false;
        }
#endif
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        return true;
    }

    bool AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
    {
#ifdef DEBUG_INFO
        if (!device || !pAllocateInfo || !pDescriptorSets)
        {
            LOG_ERROR("AllocateDescriptorSets failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkAllocateDescriptorSets failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }

    bool FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
    {
#ifdef DEBUG_INFO
        if (!device || descriptorPool == VK_NULL_HANDLE || !pDescriptorSets)
        {
            LOG_ERROR("FreeDescriptorSets failed: invalid parameters");
            return false;
        }
#endif
        VkResult result = vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
#ifdef DEBUG_INFO
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("vkFreeDescriptorSets failed, VkResult=%d", result);
            return false;
        }
#endif
        return true;
    }


    bool QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
    {
#ifdef DEBUG_INFO
        if (!queue || (!pSubmits && submitCount > 0)) { LOG_ERROR("QueueSubmit: null pointer"); return false; }
#endif
        VkResult res = vkQueueSubmit(queue, submitCount, pSubmits, fence);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "QueueSubmit");
#endif
        return true;
    }

    bool DeviceWaitIdle(VkDevice device)
    {
#ifdef DEBUG_INFO
        if (!device) { LOG_ERROR("DeviceWaitIdle: null pointer"); return false; }
#endif
        VkResult res = vkDeviceWaitIdle(device);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "DeviceWaitIdle");
#endif
        return true;
    }

    bool QueueWaitIdle(VkQueue queue)
    {
#ifdef DEBUG_INFO
        if (!queue) { LOG_ERROR("QueueWaitIdle: null pointer"); return false; }
#endif
        VkResult res = vkQueueWaitIdle(queue);
#ifdef DEBUG_INFO
        CHECK_RESULT(res, "QueueWaitIdle");
#endif
        return true;
    }

}