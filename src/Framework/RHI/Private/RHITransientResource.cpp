#include "RHITransientResource.h"
namespace RHI {

    // 构造函数
    RHITransientResource::RHITransientResource(ERHITransientResourceType type)
        : ResourceType(type)
    {
    }

    // 析构函数
    RHITransientResource::~RHITransientResource() = default;

   

    // Getter
    bool RHITransientResource::IsAcquired() const
    {
        return bAcquired;
    }

    const std::string& RHITransientResource::GetName() const
    {
        return Name;
    }

    ERHITransientResourceType RHITransientResource::GetResourceType() const
    {
        return ResourceType;
    }

    RHITransientTexture::RHITransientTexture(const std::shared_ptr<RHITexture>& inTexture)
        : RHITransientResource(ERHITransientResourceType::Texture)
        , Texture(inTexture) {
    }

    std::shared_ptr<RHITexture> RHITransientTexture::GetTexture() const { return Texture; }

    RHITransientBuffer::RHITransientBuffer(const std::shared_ptr<RHIBuffer>& inBuffer)
        : RHITransientResource(ERHITransientResourceType::Buffer)
        , Buffer(inBuffer) {
    }

    std::shared_ptr<RHIBuffer> RHITransientBuffer::GetBuffer() const { return Buffer; }



}