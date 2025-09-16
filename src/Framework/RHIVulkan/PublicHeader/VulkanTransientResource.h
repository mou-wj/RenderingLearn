#pragma once
#include "RHITransientResource.h"
#include "VulkanDevice.h"

namespace RHIVulkan {

class VulkanTransientResourceManager : public RHI::RHITransientResourceManager
{
public:
    explicit VulkanTransientResourceManager(VulkanDevice* device);
    ~VulkanTransientResourceManager() override;

    // 创建Vulkan TransientTexture
    RHI::RHITransientTextureSP CreateTransientTexture(const RHI::RHITextureDesc& desc) override;

    // 创建Vulkan TransientBuffer
    RHI::RHITransientBufferSP CreateTransientBuffer(const RHI::RHIBufferDesc& desc) override;

private:
    VulkanDevice* Device;
};

} // namespace WR::RHIVulkan