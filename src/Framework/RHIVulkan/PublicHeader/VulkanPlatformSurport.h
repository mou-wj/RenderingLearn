#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace RHIVulkan{
    
    class VulkanPlatformSupport
    {
    public:
        static bool CreateVulkanSurface(VkInstance instance, void* windowHandle, VkSurfaceKHR* surface);
        static bool GetFramebufferSize(void* windowHandle, int& width, int& height);
        static std::vector<const char*> GetPlatformWantedLayers();
        static std::vector<const char*> GetPlatformWantedExtentions();
        static std::vector<const char*> GetPlatformWantedDeviceExtentions();
    };
}