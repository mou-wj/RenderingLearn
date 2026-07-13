#pragma once

#include <vulkan/vulkan.h>
#include "VulkanRHIApi.h"


#include <vector>
#include <string>
#include <memory>
#include <queue>
#include <mutex>

namespace RHIVulkan{

class VulkanMemoryManager; // 前向声明
class VulkanQueue; // 前向声明
class VulkanSemaphoreManager; // 前向声明
class VulkanEventManager; // 前向声明
class VulkanStagingManager;// 前向声明
class VulkanCommandContext;
class VulkanFenceManager;
class VulkanRenderPassManager;
class VulkanDescriptorSetLayoutManager;
class VulkanDescriptorSetManager;
class VulkanShaderManager;
class VulkanPipelineLayoutCache;
class VulkanRHISyncPointManager;
class VulkanPresentExecutor;

// 延迟删除队列 - 用于延迟删除Vulkan对象
class VulkanDeferredDeleteQueue
{
public:
    enum class EResourceType
    {
        RenderPass,
        Buffer,
        BufferView,
        Image,
        ImageView,
        Pipeline,
        Framebuffer,
        Sampler,
        ShaderModule,
        Semaphore,
        MAX_TYPE
    };

    struct FDeferredDelete
    {
        uint64_t Handle;
        uint32_t FrameNumber;
    };

    VulkanDeferredDeleteQueue(class VulkanDevice* InDevice);
    ~VulkanDeferredDeleteQueue();

    // 入队资源
    template <typename T>
    void EnqueueResource(EResourceType Type, T Handle)
    {
        static_assert(sizeof(T) <= sizeof(uint64_t), "Vulkan resource handle size too large.");
        EnqueueGenericResource(Type, reinterpret_cast<uint64_t>(Handle));
    }

    // 释放资源
    void ReleaseResources(uint32_t FrameDelay = 2);

    // 立即清空所有资源
    void Clear();

private:
    void EnqueueGenericResource(EResourceType Type, uint64_t Handle);
    void ReleaseResource(EResourceType Type, const FDeferredDelete& Entry);

    class VulkanDevice* device_;
    std::array<std::queue<FDeferredDelete>, static_cast<size_t>(EResourceType::MAX_TYPE)> queues_;
    std::mutex queueMutex_;
    uint32_t currentFrameNumber_;

    // 删除顺序：从最不依赖的开始
    static constexpr std::array<EResourceType, 15> DeletionOrder = {
        EResourceType::Pipeline,        // Pipeline 依赖于 PipelineLayout 和 ShaderModule
        EResourceType::Framebuffer,     // Framebuffer 依赖于 RenderPass 和 ImageView
        EResourceType::ImageView,       // ImageView 依赖于 Image
        EResourceType::BufferView,      // BufferView 依赖于 Buffer
        EResourceType::RenderPass,      // RenderPass 不依赖其他延迟删除对象
        EResourceType::ShaderModule,    // ShaderModule 不依赖其他延迟删除对象
        EResourceType::Image,           // Image 不依赖其他延迟删除对象
        EResourceType::Buffer,          // Buffer 不依赖其他延迟删除对象
        EResourceType::Sampler,         // Sampler 不依赖其他延迟删除对象
        EResourceType::Semaphore,       // Semaphore 不依赖其他延迟删除对象
    };
};

class VulkanDevice
{
public:
    VulkanDevice(VulkanRHIApi* rhiApi,VkPhysicalDevice physicalDevice);

    ~VulkanDevice();
    bool Init(const std::vector<const char*>& enabledLayers,const std::vector<const char*>& enabledExtensions);
    void Destroy();
    VulkanRHIApi* GetRhiApi() const { return rhiApi_; }
    VkDevice GetHandle() const { return device_; }
    VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice_; }
    VulkanQueue* GetGraphicsQueue() const { return graphicsQueue_; }
    VulkanQueue* GetComputeQueue() const { return computeQueue_; }
    VulkanQueue* GetTransferQueue() const { return transferQueue_; }
    VulkanQueue* GetPresentQueue() const { return presentQueue_; }

    uint32_t GetGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex_; }
    uint32_t GetComputeQueueFamilyIndex() const { return computeQueueFamilyIndex_; }
    uint32_t GetTransferQueueFamilyIndex() const { return transferQueueFamilyIndex_; }
    uint32_t GetPresentQueueFamilyIndex() const { return presentQueueFamilyIndex_; }

    VulkanMemoryManager* GetMemoryManager() const { return memoryManager_; }
    VkInstance GetInstance() const { return rhiApi_->GetInstance(); }
    bool InitPresentQueue(VkSurfaceKHR Surface);
    VulkanSemaphoreManager* GetSemaphoreManager() const { return semaphoreManager_; }
	VulkanEventManager* GetEventManager() const { return eventManager_; }
	VulkanStagingManager* GetStagingManager() const { return stagingManager_; }
    VulkanCommandContext* GetGlobalCommandContext() const { return globalCommandContext_; }
    VulkanFenceManager* GetFenceManager() const { return fenceManager_; }
	VulkanRenderPassManager* GetRenderPassManager() const { return renderPassManager_; }
    VulkanDescriptorSetLayoutManager* GetDescriptorSetLayoutManager() const { return descriptorSetLayoutManager_; }
    VulkanDescriptorSetManager* GetDescriptorSetManager() const { return descriptorSetManager_; }
	VulkanShaderManager* GetShaderManager() const { return shaderManager_; }
    VulkanPipelineLayoutCache* GetPipelineLayoutCache() const { return pipelineLayoutCache_; }
    VulkanPresentExecutor* GetPresentExecutor() const { return presentExecutor_; }

    // 延迟删除资源接口
    void EnqueueRenderPassForDeletion(VkRenderPass RenderPass);
    void EnqueueBufferForDeletion(VkBuffer Buffer);
    void EnqueueImageForDeletion(VkImage Image);
    void EnqueueImageViewForDeletion(VkImageView ImageView);
    void EnqueueBufferViewForDeletion(VkBufferView BufferView);
    void EnqueuePipelineForDeletion(VkPipeline Pipeline);
    void EnqueueFramebufferForDeletion(VkFramebuffer Framebuffer);
    void EnqueueSamplerForDeletion(VkSampler Sampler);
    void EnqueueShaderModuleForDeletion(VkShaderModule ShaderModule);
    void EnqueueSemaphoreForDeletion(VkSemaphore Semaphore);

    // 释放延迟删除的资源
    void ReleaseDeferredResources(uint32_t FrameDelay = 2);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
private:
    void SelectQueueFamilies(VkPhysicalDevice physicalDevice);
    void CreateLogicalDevice(VkPhysicalDevice physicalDevice,
                             const std::vector<const char*>& extensions,
                             const std::vector<const char*>& layers);

    VulkanRHIApi *rhiApi_;

    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_{VK_NULL_HANDLE};
	VkPhysicalDeviceFeatures2 supportedFeatures2_{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures_{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };

    uint32_t queueFamilyCount = 0;
    uint32_t graphicsQueueFamilyIndex_ = UINT32_MAX;
    uint32_t computeQueueFamilyIndex_ = UINT32_MAX;
    uint32_t transferQueueFamilyIndex_ = UINT32_MAX;
    uint32_t presentQueueFamilyIndex_ = UINT32_MAX;

    VulkanQueue *graphicsQueue_;
    VulkanQueue *computeQueue_;
    VulkanQueue *transferQueue_;
    VulkanQueue *presentQueue_;

    VulkanMemoryManager* memoryManager_;
	VulkanSemaphoreManager* semaphoreManager_ = nullptr;
	VulkanEventManager* eventManager_ = nullptr;
	VulkanStagingManager* stagingManager_ = nullptr;
	VulkanCommandContext* globalCommandContext_ = nullptr;
    VulkanFenceManager* fenceManager_ = nullptr;
    VulkanRenderPassManager* renderPassManager_ = nullptr;
    VulkanDescriptorSetLayoutManager* descriptorSetLayoutManager_;
    VulkanDescriptorSetManager* descriptorSetManager_;
    VulkanShaderManager* shaderManager_;
    VulkanPipelineLayoutCache* pipelineLayoutCache_;
    VulkanDeferredDeleteQueue* deferredDeleteQueue_ = nullptr;
    VulkanPresentExecutor* presentExecutor_ = nullptr;
};

}