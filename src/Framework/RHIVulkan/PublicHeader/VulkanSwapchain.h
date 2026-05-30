#pragma once

#include "VulkanDevice.h"
#include "RHIDefine.h"
#include "VulkanSync.h"
#include <algorithm>
#include <vector>
namespace RHIVulkan{

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapchain
{
public:
    struct SwapchainDesc
    {
        void* windowHandle = nullptr; // Add window handle
        uint32_t width = 0;
        uint32_t height = 0;
        ERHIFormat preferredFormat = ERHIFormat::B8G8R8A8_UNorm;
    };

    VulkanSwapchain() = default;
    VulkanSwapchain(VulkanDevice* device, const SwapchainDesc& desc,std::vector<VkImage>& outImages);
    ~VulkanSwapchain();



    VkSwapchainKHR GetHandle() const { return swapchain_; }
    VkFormat GetFormat() const { return imageFormat_; }
    VkExtent2D GetExtent() const { return extent_; }
    VkSurfaceKHR GetSurface() const { return surface_; } // Get the surface

    bool Present(VulkanQueue* presentQueue, VulkanSemaphore* drawDoneSemaphore);
    void AcquireNextImage(VulkanSemaphore* signalSemaphore, int* imageIndex);

private:
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,VkFormat wantForamt);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height, void* windowHandle);
    void Cleanup();

    VulkanDevice* device_ = nullptr;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat imageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D extent_{};
    VkSurfaceKHR surface_ = VK_NULL_HANDLE; // Store the surface
    uint32_t currentImageIndex_ = 0;
};


}
