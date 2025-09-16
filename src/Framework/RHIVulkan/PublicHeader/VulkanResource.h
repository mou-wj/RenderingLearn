#pragma once
#include "RHIResource.h"
#include "VulkanSwapchain.h"
#include "VulkanMemory.h"
#include <vector>
using namespace RHI;

namespace RHIVulkan{

// Vulkan Texture
class VulkanTexture : public RHITexture {
public:
    VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc);
    VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc, VkImage image);
    ~VulkanTexture() override;

    VkImage GetImage() const { return Image; }
    VkImageView GetImageView() const { return ImageView; }
    VkFormat GetFormat() const { return Format; }

private:
    VkImage Image = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    VulkanDevice* Device = nullptr;
    VulkanAllocation Allocation;
};

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

private:
    VkFence Fence = VK_NULL_HANDLE;
    VulkanDevice* Device = nullptr;
};

// Vulkan Viewport (Swapchain)
class VulkanViewport : public RHIVIewport {
public:
    VulkanViewport(VulkanDevice* device, uint32_t width,uint32_t height, void* windowHandle);
    ~VulkanViewport() override;

    VulkanSwapchain* GetSwapchain() const { return Swapchain; }
    virtual void Present() override;
    virtual void Tick() override;
private:
    VulkanSwapchain* Swapchain = nullptr;
    VulkanDevice* Device = nullptr;
    void* WindowHandle;
    std::vector<VulkanTexture> swapchainTextures;
	uint32_t curPresentImageIndex = 0;
};


class VulkanRHIVertexDescState : public RHIVertexDescState
{
public:
    VulkanRHIVertexDescState(VulkanDevice* device, const RHIVertexDescStateDesc& desc);
    virtual ~VulkanRHIVertexDescState();

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
class VulkanRHIRasterizerState : public RHIRasterizerState
{
public:
    VulkanRHIRasterizerState(VulkanDevice* device, const RHIRasterizerStateDesc& desc);
    virtual ~VulkanRHIRasterizerState();

    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    void Initialize();

private:
    VulkanDevice* Device = nullptr;
};

// ----------------------------
// 颜色混合状态 Vulkan派生
// ----------------------------
class VulkanRHIColorBlendState : public RHIColorBlendState
{
public:
    VulkanRHIColorBlendState(VulkanDevice* device, const RHIColorBlendStateDesc& desc);
    virtual ~VulkanRHIColorBlendState();

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    std::vector<VkPipelineColorBlendAttachmentState> attachmentStates;

    void Initialize();

private:
    VulkanDevice* Device = nullptr;
};

// ----------------------------
// 深度模板状态 Vulkan派生
// ----------------------------
class VulkanRHIDepthStencilState : public RHIDepthStencilState
{
public:
    VulkanRHIDepthStencilState(VulkanDevice* device, const RHIDepthStencilStateDesc& desc);
    virtual ~VulkanRHIDepthStencilState();

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    void Initialize();

private:
    VulkanDevice* Device = nullptr;
};

}