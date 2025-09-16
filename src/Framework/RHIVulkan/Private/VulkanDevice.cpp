#include "VulkanDevice.h"
#include <set>
#include <stdexcept>
#include "VulkanRHIApi.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanSync.h"

namespace RHIVulkan{

VulkanDevice::VulkanDevice(VulkanRHIApi* rhiApi,
                           VkPhysicalDevice physicalDevice)
    : rhiApi_(rhiApi),
      physicalDevice_(physicalDevice),
      device_(VK_NULL_HANDLE),
      graphicsQueueFamilyIndex_(UINT32_MAX),
      computeQueueFamilyIndex_(UINT32_MAX),
      transferQueueFamilyIndex_(UINT32_MAX),
      presentQueueFamilyIndex_(UINT32_MAX),
      graphicsQueue_(nullptr),
      computeQueue_(nullptr),
      transferQueue_(nullptr),
      presentQueue_(nullptr),
      memoryManager_(nullptr)
{
    memoryManager_ = new VulkanMemoryManager(this);
}

VulkanDevice::~VulkanDevice()
{
    if (device_ != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device_, nullptr);
    }
    if (memoryManager_ != nullptr)
    {
        delete memoryManager_;
    }
    if (graphicsQueue_ != nullptr)
    {
        delete graphicsQueue_;
    }
    if (computeQueue_ != nullptr)
    {
        delete computeQueue_;
    }
    if (transferQueue_ != nullptr)
    {
        delete transferQueue_;
    }
    if (presentQueue_ != nullptr)
    {
        delete presentQueue_;
    }
}

bool VulkanDevice::Init(const std::vector<const char*>& enabledLayers,
                        const std::vector<const char*>& enabledExtensions)
{
    SelectQueueFamilies(physicalDevice_);
    CreateLogicalDevice(physicalDevice_, enabledExtensions, enabledLayers);

    return true;
}

void VulkanDevice::SelectQueueFamilies(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = queueFamilies[i];

        if ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) && graphicsQueueFamilyIndex_ == UINT32_MAX)
            graphicsQueueFamilyIndex_ = i;

        if ((props.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) && computeQueueFamilyIndex_ == UINT32_MAX)
            computeQueueFamilyIndex_ = i;

        if ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(props.queueFlags & VK_QUEUE_COMPUTE_BIT) && transferQueueFamilyIndex_ == UINT32_MAX)
            transferQueueFamilyIndex_ = i;
    }

    // fallback if no dedicated family found
    if (computeQueueFamilyIndex_ == UINT32_MAX)
        computeQueueFamilyIndex_ = graphicsQueueFamilyIndex_;
    if (transferQueueFamilyIndex_ == UINT32_MAX)
        transferQueueFamilyIndex_ = graphicsQueueFamilyIndex_;

}

void VulkanDevice::CreateLogicalDevice(VkPhysicalDevice physicalDevice,
                                       const std::vector<const char*>& extensions,
                                       const std::vector<const char*>& layers)
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        graphicsQueueFamilyIndex_,
        computeQueueFamilyIndex_,
        transferQueueFamilyIndex_,
        presentQueueFamilyIndex_
    };

    float queuePriority = 1.0f;

    for (uint32_t familyIndex : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = familyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan logical device!");
    }
    // 获取队列
    VkQueue graphicsQueue;
    vkGetDeviceQueue(device_, graphicsQueueFamilyIndex_, 0, &graphicsQueue);
    VkQueue computeQueue;
    vkGetDeviceQueue(device_, computeQueueFamilyIndex_, 0, &computeQueue);
    VkQueue transferQueue;
    vkGetDeviceQueue(device_, transferQueueFamilyIndex_, 0, &transferQueue);

    graphicsQueue_ = new VulkanQueue(this,graphicsQueue, graphicsQueueFamilyIndex_);
    computeQueue_ = new VulkanQueue(this,computeQueue, computeQueueFamilyIndex_);
    transferQueue_ = new VulkanQueue(this,transferQueue, transferQueueFamilyIndex_);
}

bool VulkanDevice::InitPresentQueue(VkSurfaceKHR Surface)
{
    VkPhysicalDevice physicalDevice = GetPhysicalDevice();
    VkSurfaceKHR surface = Surface;

     uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

     for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport && presentQueueFamilyIndex_ == UINT32_MAX)
        {
            presentQueueFamilyIndex_ = i;
        }
    }
     if (presentQueueFamilyIndex_ == UINT32_MAX)
    {
        return false;
    }
    VkQueue presentQueue;
    vkGetDeviceQueue(device_, presentQueueFamilyIndex_, 0, &presentQueue);
    presentQueue_ = new VulkanQueue(this, presentQueue, presentQueueFamilyIndex_);
    return true;
}

void VulkanDevice::Destroy()
{
    if (memoryManager_ != nullptr)
    {
        delete memoryManager_;
        memoryManager_ = nullptr;
    }

    if (device_ != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
}

}