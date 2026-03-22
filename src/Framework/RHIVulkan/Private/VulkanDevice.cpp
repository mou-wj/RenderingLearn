#include "VulkanDevice.h"
#include <set>
#include <stdexcept>
#include <array>
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
	pipelineLayoutCache_ = new VulkanPipelineLayoutCache(this);
    semaphoreManager_ = new VulkanSemaphoreManager(this);
    deferredDeleteQueue_ = new VulkanDeferredDeleteQueue(this);
}

VulkanDevice::~VulkanDevice()
{
    Destroy();
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

    // Step 1: Try to find a queue family that supports all operations (Graphics + Compute + Transfer)
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = queueFamilies[i];
        if ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            (props.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            (props.queueFlags & VK_QUEUE_TRANSFER_BIT))
        {
            graphicsQueueFamilyIndex_ = i;
            computeQueueFamilyIndex_ = i;
            transferQueueFamilyIndex_ = i;
            return; // Found ideal queue, use it for all operations
        }
    }

    // Step 2: Try to find a queue family that supports Graphics + Transfer
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = queueFamilies[i];
        if ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            (props.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            graphicsQueueFamilyIndex_ == UINT32_MAX)
        {
            graphicsQueueFamilyIndex_ = i;
            transferQueueFamilyIndex_ = i;
            break;
        }
    }

    // Step 3: Try to find a dedicated Compute queue (without Graphics)
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = queueFamilies[i];
        if ((props.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            computeQueueFamilyIndex_ == UINT32_MAX)
        {
            computeQueueFamilyIndex_ = i;
            break;
        }
    }

    // Step 4: Try to find a dedicated Transfer queue (without Graphics and Compute)
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = queueFamilies[i];
        if ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(props.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            transferQueueFamilyIndex_ == UINT32_MAX)
        {
            transferQueueFamilyIndex_ = i;
            break;
        }
    }

    // Step 5: If Graphics queue not found, find any queue with Graphics capability
    if (graphicsQueueFamilyIndex_ == UINT32_MAX)
    {
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            const auto& props = queueFamilies[i];
            if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueueFamilyIndex_ = i;
                break;
            }
        }
    }

    // Step 6: Fallback - use Graphics queue for missing operations
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

    graphicsQueue_ = new VulkanQueue(this, graphicsQueue, graphicsQueueFamilyIndex_);
    if (computeQueueFamilyIndex_ == graphicsQueueFamilyIndex_)
    {
        computeQueue_ = graphicsQueue_;
    }
    else
    {
        computeQueue_ = new VulkanQueue(this, computeQueue, computeQueueFamilyIndex_);
    }
    if (transferQueueFamilyIndex_ == graphicsQueueFamilyIndex_)
    {
        transferQueue_ = graphicsQueue_;
    }
    else if (transferQueueFamilyIndex_ == computeQueueFamilyIndex_)
    {
        transferQueue_ = computeQueue_;
    }
    else
    {
        transferQueue_ = new VulkanQueue(this, transferQueue, transferQueueFamilyIndex_);
    }
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
    vkDeviceWaitIdle(device_);

    if (globalCommandContext_ != nullptr)
    {
        delete globalCommandContext_;
        globalCommandContext_ = nullptr;
    }

    if (semaphoreManager_ != nullptr)
    {
        delete semaphoreManager_;
        semaphoreManager_ = nullptr;
    }

    if (pipelineLayoutCache_ != nullptr)
    {
        delete pipelineLayoutCache_;
        pipelineLayoutCache_ = nullptr;
    }

    if (shaderManager_ != nullptr)
    {
        delete shaderManager_;
        shaderManager_ = nullptr;
    }

    if (descriptorSetManager_ != nullptr)
    {
        delete descriptorSetManager_;
        descriptorSetManager_ = nullptr;
    }

    if (descriptorSetLayoutManager_ != nullptr)
    {
        delete descriptorSetLayoutManager_;
        descriptorSetLayoutManager_ = nullptr;
    }

    if (renderPassManager_ != nullptr)
    {
        delete renderPassManager_;
        renderPassManager_ = nullptr;
    }

    if (fenceManager_ != nullptr)
    {
        delete fenceManager_;
        fenceManager_ = nullptr;
    }

    if (stagingManager_ != nullptr)
    {
        delete stagingManager_;
        stagingManager_ = nullptr;
    }

    if (memoryManager_ != nullptr)
    {
        delete memoryManager_;
        memoryManager_ = nullptr;
    }



    
    ReleaseDeferredResources();

    std::set<VulkanQueue*> uniqueQueues;
    uniqueQueues.insert(graphicsQueue_);
    uniqueQueues.insert(computeQueue_);
    uniqueQueues.insert(transferQueue_);
    uniqueQueues.insert(presentQueue_);
    for (auto queue : uniqueQueues) {
        if (queue != nullptr)
        {
            delete queue;
        }
    }
    transferQueue_ = nullptr;
    computeQueue_ = nullptr;
    graphicsQueue_ = nullptr;
    presentQueue_ = nullptr;

    if (deferredDeleteQueue_ != nullptr)
    {
        deferredDeleteQueue_->Clear();
        delete deferredDeleteQueue_;
        deferredDeleteQueue_ = nullptr;
    }

    if (device_ != VK_NULL_HANDLE)
    {
        DestroyDevice(device_);
        device_ = VK_NULL_HANDLE;
    }
}

// ============ VulkanDeferredDeleteQueue Implementation ============

VulkanDeferredDeleteQueue::VulkanDeferredDeleteQueue(VulkanDevice* InDevice)
    : device_(InDevice),
      currentFrameNumber_(0)
{
}

VulkanDeferredDeleteQueue::~VulkanDeferredDeleteQueue()
{
    Clear();
}

void VulkanDeferredDeleteQueue::EnqueueGenericResource(EResourceType Type, uint64_t Handle)
{
    if (Handle == 0)
        return;

    std::lock_guard<std::mutex> Lock(queueMutex_);
    FDeferredDelete Entry;
    Entry.Handle = Handle;
    Entry.FrameNumber = currentFrameNumber_;
    queues_[static_cast<size_t>(Type)].push(Entry);
}

void VulkanDeferredDeleteQueue::ReleaseResource(EResourceType Type, const FDeferredDelete& Entry)
{
    if (!device_ || device_->GetHandle() == VK_NULL_HANDLE)
        return;

    VkDevice VulkanDevice = device_->GetHandle();
    switch (Type)
    {
    case EResourceType::RenderPass:
        vkDestroyRenderPass(VulkanDevice, reinterpret_cast<VkRenderPass>(Entry.Handle), nullptr);
        break;
    case EResourceType::Buffer:
        vkDestroyBuffer(VulkanDevice, reinterpret_cast<VkBuffer>(Entry.Handle), nullptr);
        break;
    case EResourceType::BufferView:
        vkDestroyBufferView(VulkanDevice, reinterpret_cast<VkBufferView>(Entry.Handle), nullptr);
        break;
    case EResourceType::Image:
    {
        VkImage Image = reinterpret_cast<VkImage>(Entry.Handle);
        device_->GetGraphicsQueue()->GetImageLayoutManager()->NotifyDeletedImage(Image);
        device_->GetComputeQueue()->GetImageLayoutManager()->NotifyDeletedImage(Image);
        device_->GetTransferQueue()->GetImageLayoutManager()->NotifyDeletedImage(Image);
        device_->GetPresentQueue()->GetImageLayoutManager()->NotifyDeletedImage(Image);

        vkDestroyImage(VulkanDevice, Image, nullptr);
        break;
    }
    case EResourceType::ImageView:
        vkDestroyImageView(VulkanDevice, reinterpret_cast<VkImageView>(Entry.Handle), nullptr);
        break;
    case EResourceType::Pipeline:
        vkDestroyPipeline(VulkanDevice, reinterpret_cast<VkPipeline>(Entry.Handle), nullptr);
        break;
    case EResourceType::Framebuffer:
        vkDestroyFramebuffer(VulkanDevice, reinterpret_cast<VkFramebuffer>(Entry.Handle), nullptr);
        break;
    case EResourceType::Sampler:
        vkDestroySampler(VulkanDevice, reinterpret_cast<VkSampler>(Entry.Handle), nullptr);
        break;
    case EResourceType::ShaderModule:
        vkDestroyShaderModule(VulkanDevice, reinterpret_cast<VkShaderModule>(Entry.Handle), nullptr);
        break;
    default:
        break;
    }
}

void VulkanDeferredDeleteQueue::ReleaseResources(uint32_t FrameDelay)
{
    std::lock_guard<std::mutex> Lock(queueMutex_);
    currentFrameNumber_++;

    // 按照删除顺序处理每个类型的队列
    for (EResourceType Type : DeletionOrder)
    {
        auto& queue = queues_[static_cast<size_t>(Type)];
        while (!queue.empty())
        {
            const FDeferredDelete& Entry = queue.front();

            ReleaseResource(Type, Entry);
            queue.pop();


        }
    }
}

void VulkanDeferredDeleteQueue::Clear()
{
    std::lock_guard<std::mutex> Lock(queueMutex_);

    // 按照删除顺序立即释放所有待处理资源
    for (EResourceType Type : DeletionOrder)
    {
        auto& queue = queues_[static_cast<size_t>(Type)];
        while (!queue.empty())
        {
            const FDeferredDelete& Entry = queue.front();
            ReleaseResource(Type, Entry);
            queue.pop();
        }
    }
}
void VulkanDevice::EnqueueRenderPassForDeletion(VkRenderPass RenderPass)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::RenderPass, RenderPass);
}

void VulkanDevice::EnqueueBufferForDeletion(VkBuffer Buffer)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::Buffer, Buffer);
}

void VulkanDevice::EnqueueImageForDeletion(VkImage Image)
{
    if (deferredDeleteQueue_)
    {
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::Image, Image);
		// 通知 RenderPassManager 相关的 Framebuffer 可能需要更新
        if (renderPassManager_)
        {
            renderPassManager_->NotifyDeletedImage(Image);
        }
            			
    }

}

void VulkanDevice::EnqueueImageViewForDeletion(VkImageView ImageView)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::ImageView, ImageView);
}

void VulkanDevice::EnqueueBufferViewForDeletion(VkBufferView BufferView)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::BufferView, BufferView);
}

void VulkanDevice::EnqueuePipelineForDeletion(VkPipeline Pipeline)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::Pipeline, Pipeline);
}


void VulkanDevice::EnqueueFramebufferForDeletion(VkFramebuffer Framebuffer)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::Framebuffer, Framebuffer);
}

void VulkanDevice::EnqueueSamplerForDeletion(VkSampler Sampler)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::Sampler, Sampler);
}

void VulkanDevice::EnqueueShaderModuleForDeletion(VkShaderModule ShaderModule)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->EnqueueResource(VulkanDeferredDeleteQueue::EResourceType::ShaderModule, ShaderModule);
}


void VulkanDevice::ReleaseDeferredResources(uint32_t FrameDelay)
{
    if (deferredDeleteQueue_)
        deferredDeleteQueue_->ReleaseResources(FrameDelay);
}

}