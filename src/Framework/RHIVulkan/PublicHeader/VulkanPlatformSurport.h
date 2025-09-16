#pragma once
#include <vulkan/vulkan.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif

namespace RHIVulkan{
    
    class VulkanPlatformSupport
    {
    public:
        static bool CreateVulkanSurface(VkInstance instance, void* windowHandle, VkSurfaceKHR* surface);

    };
}