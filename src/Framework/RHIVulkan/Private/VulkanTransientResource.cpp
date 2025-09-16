#include "VulkanTransientResource.h"
#include "VulkanResource.h"

namespace RHIVulkan {

VulkanTransientResourceManager::VulkanTransientResourceManager(VulkanDevice* device)
    : Device(device)
{
}

VulkanTransientResourceManager::~VulkanTransientResourceManager() = default;

RHI::RHITransientTextureSP VulkanTransientResourceManager::CreateTransientTexture(const RHI::RHITextureDesc& desc)
{
    // 假设VulkanTexture继承自RHITexture
    auto vulkanTexture = std::make_shared<VulkanTexture>(Device, desc);
    return std::make_shared<RHI::RHITransientTexture>(vulkanTexture);
}

RHI::RHITransientBufferSP VulkanTransientResourceManager::CreateTransientBuffer(const RHI::RHIBufferDesc& desc)
{
    // 假设VulkanBuffer继承自RHIBuffer
    auto vulkanBuffer = std::make_shared<VulkanBuffer>(Device, desc);
    return std::make_shared<RHI::RHITransientBuffer>(vulkanBuffer);
}

} // namespace WR::RHIVulkan