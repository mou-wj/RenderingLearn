#pragma once
#include "RHIResource.h"

#include "VulkanMemory.h"
#include "VulkanDevice.h"
#include <vector>
#include <memory>
#include <deque>
using namespace RHI;

namespace RHIVulkan{
class VulkanCommandContext;

struct VulkanTextureView
{
    VulkanTextureView()
        : View(VK_NULL_HANDLE)
        , Image(VK_NULL_HANDLE)
        , ViewId(0)
    {
    }

    // 创建 ImageView
    bool Create(VulkanDevice* Device,
        VkImage InImage,
        VkImageViewType ViewType,
        VkImageAspectFlags AspectFlags,
        VkFormat Format,
        uint32_t FirstMip,
        uint32_t NumMips,
        uint32_t ArraySliceIndex,
        uint32_t NumArraySlices,
        bool bUseIdentitySwizzle = false,
        VkImageUsageFlags ImageUsageFlags = 0)
    {
        assert(InImage != VK_NULL_HANDLE);

        Image = InImage;

        // VkImageViewCreateInfo
        VkImageViewCreateInfo ViewInfo{};
        ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewInfo.image = Image;
        ViewInfo.viewType = ViewType;
        ViewInfo.format = Format;

        ViewInfo.subresourceRange.aspectMask = AspectFlags;
        ViewInfo.subresourceRange.baseMipLevel = FirstMip;
        ViewInfo.subresourceRange.levelCount = NumMips;
        ViewInfo.subresourceRange.baseArrayLayer = ArraySliceIndex;
        ViewInfo.subresourceRange.layerCount = NumArraySlices;

        // UE 支持 Identity Swizzle
        if (bUseIdentitySwizzle)
        {
            ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        }
        else
        {
            ViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            ViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            ViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            ViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        }

        VkResult Result = vkCreateImageView(Device->GetHandle(), &ViewInfo, nullptr, &View);
        assert(Result == VK_SUCCESS && "Failed to create VkImageView");

        // 简单 ID 用于 debug / hash
        static uint32_t NextViewId = 1;
        ViewId = NextViewId++;
        return Result == VK_SUCCESS;
    }

    // 销毁 ImageView
    void Destroy(VulkanDevice* Device)
    {
        if (View != VK_NULL_HANDLE)
        {
            Device->EnqueueImageViewForDeletion(View);
            View = VK_NULL_HANDLE;
        }

        Image = VK_NULL_HANDLE;
        ViewId = 0;
    }

public:
    VkImageView View;
    VkImage Image;
    uint32_t ViewId;
};

struct VulkanBufferView
{
    VulkanBufferView()
        : View(VK_NULL_HANDLE)
        , Buffer(VK_NULL_HANDLE)
        , ViewId(0)
        , Format(VK_FORMAT_UNDEFINED)
        , Size(0)
        , Offset(0)
    {
    }

    // 创建 BufferView
    bool Create(VulkanDevice* Device,
        VkBuffer InBuffer,
        VkFormat InFormat,
        VkDeviceSize Offset,
        VkDeviceSize Size)
    {
        assert(InBuffer != VK_NULL_HANDLE);

        Buffer = InBuffer;
        Format = InFormat;

        // VkBufferViewCreateInfo
        VkBufferViewCreateInfo ViewInfo{};
        ViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        ViewInfo.buffer = Buffer;
        ViewInfo.format = Format;
        ViewInfo.offset = Offset;
        ViewInfo.range = Size;
        this->Size = Size;
        this->Offset = Offset;

        VkResult Result = vkCreateBufferView(Device->GetHandle(), &ViewInfo, nullptr, &View);
        assert(Result == VK_SUCCESS && "Failed to create VkBufferView");

        // 简单 ID 用于 debug / hash
        static uint32_t NextViewId = 1;
        ViewId = NextViewId++;
        return Result == VK_SUCCESS;
    }

    // 销毁 BufferView
    void Destroy(VulkanDevice* Device)
    {
        if (View != VK_NULL_HANDLE)
        {
            Device->EnqueueBufferViewForDeletion(View);
            View = VK_NULL_HANDLE;
        }

        Buffer = VK_NULL_HANDLE;
        Format = VK_FORMAT_UNDEFINED;
        ViewId = 0;
    }

public:
    VkBufferView View;
    VkBuffer Buffer;
    uint32_t ViewId;
    VkFormat Format;
    VkDeviceSize Size;
    VkDeviceSize Offset;
};

class VulkanViewBase{
public:
    virtual void Invalidate() = 0;
    virtual ~VulkanViewBase() = default;
protected:
    bool IsValid = true;
};

// Vulkan Texture
class VulkanTexture : public RHITexture {
public:
    VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc);
    VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc, VkImage image);
    ~VulkanTexture() override;

    VkImage GetImage() const { return Image; }
    VkImageView GetImageView() const { return DefaltView.View; }
    VkFormat GetFormat() const { return Format; }
	VkImageAspectFlags GetAspectFlags() const { return ImageAspectFlags; }
	VulkanAllocation& GetAllocation() { return Allocation; }


    // View management
    void AttachView(VulkanViewBase* view);
    void DetachView(VulkanViewBase* view);
private:
    void DetermineDefaultLayout(ERHITextureCreateFlags Flags, VkImageLayout &Layout,ERHIResourceAccess &Access);

    VkImage Image = VK_NULL_HANDLE;
    VulkanTextureView DefaltView;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    VulkanDevice* Device = nullptr;
    VulkanAllocation Allocation;
    bool owner = true;
	VkImageAspectFlags ImageAspectFlags = 0;

    std::vector<VulkanViewBase*> views;
};
using VulkanTextureSP = std::shared_ptr<VulkanTexture>;

// Vulkan Buffer
class VulkanBuffer : public RHIBuffer {
public:
    VulkanBuffer(VulkanDevice* device, const RHIBufferDesc& desc);
    ~VulkanBuffer() override;

    const VkBuffer& GetHandle() const { return Buffer; }
    VkDeviceSize GetSize() const { return Size; }

    // View management
    void AttachView(VulkanViewBase* view);
    void DetachView(VulkanViewBase* view);

private:
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceSize Size = 0;
    VulkanDevice* Device = nullptr;
    VulkanAllocation Allocation;

    std::vector<VulkanViewBase*> views;
};

// --------------------------------------------------
// Vulkan Shader Resource View
// --------------------------------------------------
class VulkanShaderResourceView : public RHIShaderResourceView, public VulkanViewBase
{
public:
    // Texture SRV 构造
    VulkanShaderResourceView(VulkanDevice* device, RHIViewableResource* Resource, const RHITexSRVCreateInfo& SRVInfo);
    // Buffer SRV 构造
    VulkanShaderResourceView(VulkanDevice* device, RHIViewableResource* Resource, const RHIBufferSRVCreateInfo& SRVInfo);

    ~VulkanShaderResourceView() override;

    // Vulkan 句柄访问
    const VulkanTextureView& GetTextureView() const;
    const VulkanBufferView& GetBufferView() const;

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

    // VulkanViewBase override
    void Invalidate() override;

private:
    void CreateTextureView(const RHITexSRVCreateInfo& SRVInfo);
    void CreateBufferView(const RHIBufferSRVCreateInfo& SRVInfo);
    void DestroyView();

private:
    VulkanDevice* Device = nullptr;
    RHIViewableResource* ResourcePtr = nullptr;

    // 存储 view 对象（而非原生句柄）
    VulkanTextureView TextureView;
    VulkanBufferView BufferView;

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
class VulkanUnorderedAccessView : public RHIUnorderedAccessView, public VulkanViewBase
{
public:
    VulkanUnorderedAccessView(VulkanDevice* device, RHIViewableResource* Resource, const RHITexUAVCreateInfo& UAVInfo);
    VulkanUnorderedAccessView(VulkanDevice* device, RHIViewableResource* Resource, const RHIBufferUAVCreateInfo& UAVInfo);

    ~VulkanUnorderedAccessView() override;

    // Vulkan 句柄访问
    const VulkanTextureView& GetTextureView() const;
    const VulkanBufferView& GetBufferView() const;

    bool IsTexture() const { return ResourceType == EResourceType::Texture; }
    bool IsBuffer() const { return ResourceType == EResourceType::Buffer; }
    VkFormat GetFormat() const { return Format; }
    uint32_t GetBaseMipLevel() const { return BaseMipLevel; }
    uint32_t GetMipLevelCount() const { return MipLevelCount; }
    uint32_t GetBaseArrayLayer() const { return BaseArrayLayer; }
    uint32_t GetLayerCount() const { return LayerCount; }
    VkDescriptorType GetDescriptorType() const { return DescriptorType; }
    const std::string& GetDebugName() const { return DebugName; }

    // VulkanViewBase override
    void Invalidate() override;

private:
    void CreateTextureView(const RHITexUAVCreateInfo& UAVInfo);
    void CreateBufferView(const RHIBufferUAVCreateInfo& UAVInfo);
    void DestroyView();

private:
    VulkanDevice* Device = nullptr;
    RHIViewableResource* ResourcePtr = nullptr;

    // 存储 view 对象（而非原生句柄）
    VulkanTextureView TextureView;
    VulkanBufferView BufferView;

    enum class EResourceType { Texture, Buffer } ResourceType;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    uint32_t BaseMipLevel = 0;
    uint32_t MipLevelCount = 1;
    uint32_t BaseArrayLayer = 0;
    uint32_t LayerCount = 1;

    VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    std::string DebugName;
};


// Vulkan Ring Buffer - 用于动态分配小块内存的环形缓冲区
class VulkanRingBuffer
{
public:
    VulkanRingBuffer(VulkanDevice* device, uint64_t totalSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memPropertyFlags);
    ~VulkanRingBuffer();

    // 分配内存空间
    uint64_t AllocateMemory(uint64_t size, uint32_t alignment, VulkanCommandBuffer* cmdBuffer);

    // 获取缓冲区信息
    uint32_t GetBufferOffset() const { return Allocation.GetOffset(); }
    VkDeviceAddress GetBufferAddress() const { return BufferAddress; }
    VkBuffer GetHandle() const { return Allocation.GetBufferHandle(); }
    void* GetMappedPointer() { return Allocation.GetMappedPointer(); }
    VulkanAllocation& GetAllocation() { return Allocation; }
    const VulkanAllocation& GetAllocation() const { return Allocation; }

private:
    uint64_t BufferSize;
    uint64_t BufferOffset;
    VkDeviceAddress BufferAddress;
    uint32_t MinAlignment;
    VulkanAllocation Allocation;
    VulkanDevice* Device;

    // 用于环绕分配的同步
    VulkanCommandBuffer* FenceCmdBuffer = nullptr;
    uint64_t FenceCounter = 0;

    uint64_t WrapAroundAllocateMemory(uint64_t size, uint32_t alignment, VulkanCommandBuffer* cmdBuffer);
};

// Vulkan Loose Uniform Buffer Uploader - 用于上传uniform buffer数据的工具
class VulkanLooseUniformDataUploader
{
public:
    VulkanLooseUniformDataUploader(VulkanDevice* device);
    ~VulkanLooseUniformDataUploader();

    // 获取CPU映射指针
    uint8_t* GetCPUMappedPointer() { return static_cast<uint8_t*>(CPUBuffer->GetMappedPointer()); }

    // 分配内存
    uint64_t AllocateMemory(uint64_t size, uint32_t alignment, VulkanCommandBuffer* cmdBuffer)
    {
        return CPUBuffer->AllocateMemory(size, alignment, cmdBuffer);
    }

    // 获取缓冲区信息
    const VulkanAllocation& GetCPUBufferAllocation() const { return CPUBuffer->GetAllocation(); }
    VkBuffer GetCPUBufferHandle() const { return CPUBuffer->GetHandle(); }
    uint32_t GetCPUBufferOffset() const { return CPUBuffer->GetBufferOffset(); }
    VkDeviceAddress GetCPUBufferAddress() const { return CPUBuffer->GetBufferAddress(); }

private:
    VulkanRingBuffer* CPUBuffer;
    friend class VulkanCommandContext; // 允许命令上下文访问
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

class VulkanSwapchain;
class VulkanSemaphore;
class VulkanRHISyncPoint;
class VulkanCommandContext;
class VulkanCommandBuffer;
class VulkanQueue;
// Vulkan RHISwapchain implementation
class VulkanRHISwapchain : public RHISwapchain {
public:
    VulkanRHISwapchain(VulkanDevice* device, uint32_t width, uint32_t height, void* windowHandle, ERHIFormat format);
    ~VulkanRHISwapchain() override;

    VulkanSwapchain* GetSwapchain() const { return Swapchain; }
    RHISwapchainSlot AcquireNextSlot() override;
    void Resize(uint32_t Width, uint32_t Height) override;
    void Present(VulkanQueue* presentQueue, const RHI::RHIWaitInfo& waitInfo);
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
    std::vector<VulkanRHISyncPoint*> acquireSemaphores;
    std::queue<VulkanSemaphore*> presentSemaphores;
    int currentBackBufferIndex = -1;  // 渲染用
    int currentIndex = 0; // Present用
    int currentSemaphoreIndex = -1; // 当前已Acquire但未Present的信号量槽位
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