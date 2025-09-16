#pragma once

#include "VulkanDevice.h"
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
        VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_UNORM;
    };

    VulkanSwapchain() = default;
    VulkanSwapchain(VulkanDevice* device, const SwapchainDesc& desc);
    ~VulkanSwapchain();



    VkSwapchainKHR GetHandle() const { return swapchain_; }
    VkFormat GetFormat() const { return imageFormat_; }
    VkExtent2D GetExtent() const { return extent_; }
    const std::vector<VkImage>& GetImages() const { return images_; }
    VkSurfaceKHR GetSurface() const { return surface_; } // Get the surface

    bool Present(uint32_t imageIndex, VkSemaphore waitSemaphore);

private:
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height, void* windowHandle);
    void Cleanup();

    VulkanDevice* device_ = nullptr;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat imageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D extent_{};

    std::vector<VkImage> images_;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE; // Store the surface
};


}
