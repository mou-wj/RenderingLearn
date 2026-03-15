#include "VulkanDevice.h"
#include <set>
#include <stdexcept>
#include "VulkanRHIApi.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanSync.h"
#include "VulkanFuncWrapper.h"
#include "VulkanCommandContex.h"
#include "VulkanResource.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSets.h"
#include "VulkanShader.h"
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
	stagingManager_ = new VulkanStagingManager(this, memoryManager_);

    fenceManager_ = new VulkanFenceManager(this);
    renderPassManager_ = new VulkanRenderPassManager(this);
    descriptorSetLayoutManager_ = new VulkanDescriptorSetLayoutManager(this);
    descriptorSetManager_ = new VulkanDescriptorSetManager(this, descriptorSetLayoutManager_);
	shaderManager_ = new VulkanShaderManager(this);
	pipelineStateCache_ = new VulkanPipelineLayoutCache(this);
    semaphoreManager_ = new VulkanSemaphoreManager(this);
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
    globalCommandContext_ = new VulkanCommandContext(this, graphicsQueue_);
    return true;
}

void VulkanDevice::SelectQueueFamilies(VkPhysicalDevice physicalDevice)
{
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

    CreateDevice(physicalDevice, &createInfo,&device_);
    
    // 获取队列
    VkQueue graphicsQueue;
    GetDeviceQueue(device_, graphicsQueueFamilyIndex_, 0, &graphicsQueue);
    VkQueue computeQueue;
    GetDeviceQueue(device_, computeQueueFamilyIndex_, 0, &computeQueue);
    VkQueue transferQueue;
    GetDeviceQueue(device_, transferQueueFamilyIndex_, 0, &transferQueue);

    graphicsQueue_ = new VulkanQueue(this,graphicsQueue, graphicsQueueFamilyIndex_);
    computeQueue_ = new VulkanQueue(this,computeQueue, computeQueueFamilyIndex_);
    transferQueue_ = new VulkanQueue(this,transferQueue, transferQueueFamilyIndex_);
}

bool VulkanDevice::InitPresentQueue(VkSurfaceKHR Surface)
{
    auto CheckPresentSupport = [this](VulkanQueue* queue, VkSurfaceKHR surface)  {
        if (presentQueue_) return;
        VkBool32 surport = false;
        GetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, queue->GetFamilyIndex(), surface, &surport);
        if (surport) {
            presentQueue_ = queue;
			presentQueueFamilyIndex_ = queue->GetFamilyIndex();
        }
    };
    CheckPresentSupport(graphicsQueue_, Surface);
    CheckPresentSupport(computeQueue_, Surface);
    CheckPresentSupport(transferQueue_, Surface);
    if (!presentQueue_) return false;
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
        DestroyDevice(device_);
        device_ = VK_NULL_HANDLE;
    }
}

}