#pragma once
#include "RHIResource.h"

#include "VulkanMemory.h"
#include <vector>
#include <memory>
#include <deque>
using namespace RHI;

namespace RHIVulkan{
class VulkanCommandContext;
// Vulkan Texture
class VulkanTexture : public RHITexture {
public:
    VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc);
    VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc, VkImage image);
    ~VulkanTexture() override;

    VkImage GetImage() const { return Image; }
    VkImageView GetImageView() const { return ImageView; }
    VkFormat GetFormat() const { return Format; }
	VulkanAllocation& GetAllocation() { return Allocation; }

    void InitialImageState(VulkanCommandContext* context, VkImageLayout layout);

private:
    VkImageLayout DetermineDefaultLayout(ERHITextureCreateFlags Flags);

    VkImage Image = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    VulkanDevice* Device = nullptr;
    VulkanAllocation Allocation;
    bool owner = true;
    VkImageLayout DefaultLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};
using VulkanTextureSP = std::shared_ptr<VulkanTexture>;

// Vulkan Buffer
class VulkanBuffer : public RHIBuffer {
public:
    VulkanBuffer(VulkanDevice* device, const RHIBufferDesc& desc);
    ~VulkanBuffer() override;

    VkBuffer GetBuffer() const { return Buffer; }
    VkDeviceSize GetSize() const { return Size; }

private:
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceSize Size = 0;
    VulkanDevice* Device = nullptr;
    VulkanAllocation Allocation;
};

// --------------------------------------------------
   // Vulkan Shader Resource View
   // --------------------------------------------------
class VulkanShaderResourceView : public RHIShaderResourceView
{
public:
    // Texture SRV 构造
    VulkanShaderResourceView(VulkanDevice* device, RHIViewableResource* Resource, const RHITexSRVCreateInfo& SRVInfo);
    // Buffer SRV 构造
    VulkanShaderResourceView(VulkanDevice* device, RHIViewableResource* Resource, const RHIBufferSRVCreateInfo& SRVInfo);

    ~VulkanShaderResourceView() override;

    // Vulkan 句柄访问
    VkImageView GetImageView() const { return ImageView; }
    VkBufferView GetBufferView() const { return BufferView; }

    // 元信息查询
    bool IsTexture() const { return ResourceType == EResourceType::Texture; }
    bool IsBuffer() const { return ResourceType == EResourceType::Buffer; }
    VkFormat GetFormat() const { return Format; }
    uint32_t GetBaseMipLevel() const { return BaseMipLevel; }
    uint32_t GetMipLevelCount() const { return MipLevelCount; }
    uint32_t GetBaseArrayLayer() const { return BaseArrayLayer; }
    uint32_t GetLayerCount() const { return LayerCount; }
    VkDescriptorType GetDescriptorType() const { return DescriptorType; }
    const std::string& GetDebugName() const { return DebugName; }

private:
    void CreateTextureView(const RHITexSRVCreateInfo& SRVInfo);
    void CreateBufferView(const RHIBufferSRVCreateInfo& SRVInfo);
    void DestroyView();

private:
    VulkanDevice* Device = nullptr;
    RHIViewableResource* ResourcePtr = nullptr;

    // Vulkan句柄
    VkImageView ImageView = VK_NULL_HANDLE;
    VkBufferView BufferView = VK_NULL_HANDLE;

    // 类型信息
    enum class EResourceType { Texture, Buffer } ResourceType;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    uint32_t BaseMipLevel = 0;
    uint32_t MipLevelCount = 1;
    uint32_t BaseArrayLayer = 0;
    uint32_t LayerCount = 1;

    // Descriptor 信息
    VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    // 可选调试名称
    std::string DebugName;
};


// --------------------------------------------------
// Vulkan Unordered Access View
// --------------------------------------------------
class VulkanUnorderedAccessView : public RHIUnorderedAccessView
{
public:
    VulkanUnorderedAccessView(VulkanDevice* device, RHIViewableResource* Resource, const RHITexUAVCreateInfo& UAVInfo);
    VulkanUnorderedAccessView(VulkanDevice* device, RHIViewableResource* Resource, const RHIBufferUAVCreateInfo& UAVInfo);

    ~VulkanUnorderedAccessView() override;

    VkImageView GetImageView() const { return ImageView; }
    VkBufferView GetBufferView() const { return BufferView; }

    bool IsTexture() const { return ResourceType == EResourceType::Texture; }
    bool IsBuffer() const { return ResourceType == EResourceType::Buffer; }
    VkFormat GetFormat() const { return Format; }
    uint32_t GetBaseMipLevel() const { return BaseMipLevel; }
    uint32_t GetMipLevelCount() const { return MipLevelCount; }
    uint32_t GetBaseArrayLayer() const { return BaseArrayLayer; }
    uint32_t GetLayerCount() const { return LayerCount; }
    VkDescriptorType GetDescriptorType() const { return DescriptorType; }
    const std::string& GetDebugName() const { return DebugName; }

private:
    void CreateTextureView(const RHITexUAVCreateInfo& UAVInfo);
    void CreateBufferView(const RHIBufferUAVCreateInfo& UAVInfo);
    void DestroyView();

private:
    VulkanDevice* Device = nullptr;
    RHIViewableResource* ResourcePtr = nullptr;

    VkImageView ImageView = VK_NULL_HANDLE;
    VkBufferView BufferView = VK_NULL_HANDLE;

    enum class EResourceType { Texture, Buffer } ResourceType;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    uint32_t BaseMipLevel = 0;
    uint32_t MipLevelCount = 1;
    uint32_t BaseArrayLayer = 0;
    uint32_t LayerCount = 1;

    VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    std::string DebugName;
};

// Vulkan Sampler
class VulkanSampler : public RHISampler {
public:
    VulkanSampler(VulkanDevice* device, const RHISamplerDesc& desc) : RHISampler(desc), Device(device) {}
    ~VulkanSampler() override = default;

    VkSampler GetSampler() const { return Sampler; }

private:
    VkSampler Sampler = VK_NULL_HANDLE;
    VulkanDevice* Device = nullptr;
};

// Vulkan Fence
class VulkanFence : public RHIFence {
public:
    VulkanFence(VulkanDevice* device);
    ~VulkanFence() override = default;

    // 新增：判断Fence是否已完成
    bool IsSignaled() const;
    void Reset();

private:
    VkFence Fence = VK_NULL_HANDLE;
    VulkanDevice* Device = nullptr;
};

class VulkanFenceManager
{
public:
    VulkanFenceManager(VulkanDevice* device);
    ~VulkanFenceManager();

    // 获取一个可用 fence
    VulkanFence* AcquireFence();

    // 标记 fence 已使用（通常提交命令后）
    void ReleaseFence(VulkanFence* fence);

    // 检查 fence 并回收已完成的 fence
    void GarbageCollect();

private:
    VulkanDevice* device = nullptr;

    // 所有 fence 对象
    std::vector<std::unique_ptr<VulkanFence>> allFences;

    // 空闲 fence 可供分配
    std::deque<VulkanFence*> availableFences;

    // 已提交 fence，等待 GPU 完成
    std::deque<VulkanFence*> pendingFences;
};


class VulkanSwapchain;
class VulkanSemaphore;
class VulkanCommandContext;
class VulkanCommandBuffer;
class VulkanQueue;
// Vulkan Viewport (Swapchain)
class VulkanViewport : public RHIViewport {
public:
    VulkanViewport(VulkanDevice* device, uint32_t width, uint32_t height, void* windowHandle, ERHIFormat format);
    ~VulkanViewport() override;

    VulkanSwapchain* GetSwapchain() const { return Swapchain; }
    void Present(VulkanCommandContext* context, VulkanCommandBuffer* commandBuffer,VulkanQueue* queue,VulkanQueue* presentQueue);
    virtual void Tick() override;
    VulkanTextureSP GetBackTexture();

private:
    void CreateSwapchain();
    void DestroySwapchain();

    VulkanSwapchain* Swapchain = nullptr;
    VulkanDevice* Device = nullptr;
    void* WindowHandle;
    uint32_t Width;
    uint32_t Height;
    ERHIFormat Format;

    std::vector<VulkanTextureSP> backBufferTextures; // 所有 backbuffers
    std::vector<VulkanSemaphore*> backBufferRenderDoneSemaphores; // 每个backbuffer对应的渲染完成信号
    VulkanSemaphore* acquireSemaphore = nullptr;

    int currentBackBufferIndex = -1;  // 渲染用

    std::vector<VkImage> swapchainImages_;
};


class VulkanVertexDescState : public RHIVertexDescState
{
public:
    VulkanVertexDescState(VulkanDevice* device, const RHIVertexDescStateDesc& desc);
    virtual ~VulkanVertexDescState();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    void Initialize();
private:
    VulkanDevice* Device = nullptr;
};



// ----------------------------
// 光栅化状态 Vulkan派生
// ----------------------------
class VulkanRasterizerState : public RHIRasterizerState
{
public:
    VulkanRasterizerState(VulkanDevice* device, const RHIRasterizerStateDesc& desc);
    virtual ~VulkanRasterizerState();

    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    void Initialize();

private:
    VulkanDevice* Device = nullptr;
};

// ----------------------------
// 颜色混合状态 Vulkan派生
// ----------------------------
class VulkanColorBlendState : public RHIColorBlendState
{
public:
    VulkanColorBlendState(VulkanDevice* device, const RHIColorBlendStateDesc& desc);
    virtual ~VulkanColorBlendState();

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    std::vector<VkPipelineColorBlendAttachmentState> attachmentStates;

    void Initialize();

private:
    VulkanDevice* Device = nullptr;
};

// ----------------------------
// 深度模板状态 Vulkan派生
// ----------------------------
class VulkanDepthStencilState : public RHIDepthStencilState
{
public:
    VulkanDepthStencilState(VulkanDevice* device, const RHIDepthStencilStateDesc& desc);
    virtual ~VulkanDepthStencilState();

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    void Initialize();

private:
    VulkanDevice* Device = nullptr;
};

}