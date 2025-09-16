#include "VulkanPlatformSurport.h"
#include "vulkan/vulkan_win32.h"



namespace RHIVulkan{
    
    bool VulkanPlatformSupport::CreateVulkanSurface(VkInstance instance, void* windowHandle, VkSurfaceKHR* surface)
    {
#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window((GLFWwindow*)windowHandle);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, surface) != VK_SUCCESS)
        {
            return false;
        }
        return true;
#else
        Log::Error("Unsupported platform!");
        return false;
#endif
    }

}