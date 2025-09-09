#pragma once
#include "RHIResource.h"
#include <memory>

namespace RHI {

// TransientResource基类
class RHITransientResource
{
public:
    explicit RHITransientResource(const RHIResourceSP& resource)
        : Resource(resource) {}
    virtual ~RHITransientResource() = default;

    RHIResourceSP GetResource() const { return Resource; }

protected:
    RHIResourceSP Resource;
};

// TransientTexture
class RHITransientTexture : public RHITransientResource
{
public:
    explicit RHITransientTexture(const RHITextureSP& texture)
        : RHITransientResource(texture) {}
    virtual ~RHITransientTexture() = default;

    RHITextureSP GetTexture() const { return std::static_pointer_cast<RHITexture>(Resource); }
};

// TransientBuffer
class RHITransientBuffer : public RHITransientResource
{
public:
    explicit RHITransientBuffer(const RHIBufferSP& buffer)
        : RHITransientResource(buffer) {}
    virtual ~RHITransientBuffer() = default;

    RHIBufferSP GetBuffer() const { return std::static_pointer_cast<RHIBuffer>(Resource); }
};

using RHITransientTextureSP = std::shared_ptr<RHITransientTexture>;
using RHITransientBufferSP = std::shared_ptr<RHITransientBuffer>;


// TransientResourceManager基类
class RHITransientResourceManager
{
public:
    virtual ~RHITransientResourceManager() = default;

    // 创建TransientTexture（纯虚函数）
    virtual RHITransientTextureSP CreateTransientTexture(const RHITextureDesc& desc) = 0;

    // 创建TransientBuffer（纯虚函数）
    virtual RHITransientBufferSP CreateTransientBuffer(const RHIBufferDesc& desc) = 0;
};

} //