#include "VulkanTransientResource.h"
#include "VulkanResource.h"

namespace RHIVulkan {

VulkanTransientResourceManager::VulkanTransientResourceManager(VulkanDevice* device)
    : Device(device)
{
}

VulkanTransientResourceManager::~VulkanTransientResourceManager() {

}

RHI::RHITransientTextureSP VulkanTransientResourceManager::CreateTransientTexture(const RHI::RHITextureDesc& desc)
{
    // 假设VulkanTexture继承自RHITexture
    auto vulkanTexture = std::make_shared<VulkanTexture>(Device, desc);
    return std::dynamic_pointer_cast<RHI::RHITransientTexture>(vulkanTexture);
}

RHI::RHITransientBufferSP VulkanTransientResourceManager::CreateTransientBuffer(const RHI::RHIBufferDesc& desc)
{
    // 假设VulkanBuffer继承自RHIBuffer
    auto vulkanBuffer = std::make_shared<VulkanBuffer>(Device, desc);
    return std::dynamic_pointer_cast<RHI::RHITransientBuffer>(vulkanBuffer);
}

void VulkanTransientResourceManager::ReleaseTransientTexture(const RHITransientTextureSP& texture) 
{

}
void VulkanTransientResourceManager::ReleaseTransientBuffer(const RHITransientBufferSP& buffer)
{

}

} // namespace WR::RHIVulkan