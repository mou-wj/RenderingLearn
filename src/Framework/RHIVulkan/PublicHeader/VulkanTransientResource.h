#pragma once
#include "RHITransientResource.h"
#include "VulkanDevice.h"

namespace RHIVulkan {

class RHIVULKAN_API VulkanTransientResourceManager
{
public:
    explicit VulkanTransientResourceManager(VulkanDevice* device);
    ~VulkanTransientResourceManager();

    // 创建Vulkan TransientTexture
    RHI::RHITransientTextureSP CreateTransientTexture(const RHI::RHITextureDesc& desc);

    // 创建Vulkan TransientBuffer
    RHI::RHITransientBufferSP CreateTransientBuffer(const RHI::RHIBufferDesc& desc) ;

private:
    VulkanDevice* Device;
};

} // namespace WR::RHIVulkan