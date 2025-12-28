#pragma once
#include "RHIResource.h"
#include <memory>

namespace RHI {

// TransientResource基类
class RHI_API RHITransientResource
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
class RHI_API RHITransientTexture : public RHITransientResource
{
public:
    explicit RHITransientTexture(const RHITextureSP& texture)
        : RHITransientResource(texture) {}
    virtual ~RHITransientTexture() = default;

    RHITextureSP GetTexture() const { return std::static_pointer_cast<RHITexture>(Resource); }
};

// TransientBuffer
class RHI_API RHITransientBuffer : public RHITransientResource
{
public:
    explicit RHITransientBuffer(const RHIBufferSP& buffer)
        : RHITransientResource(buffer) {}
    virtual ~RHITransientBuffer() = default;

    RHIBufferSP GetBuffer() const { return std::static_pointer_cast<RHIBuffer>(Resource); }
};

using RHITransientTextureSP = std::shared_ptr<RHITransientTexture>;
using RHITransientBufferSP = std::shared_ptr<RHITransientBuffer>;


struct RHI_API RHITransientInfo {

    union
    {
        class RHIResource* Resource = nullptr;
        class RHITexture* Texture;
        class RHIBuffer* Buffer;
        class RHIUnorderedAccessView* UAV;
    };

    enum class EType 
    {
        Unknown,
        Texture,
        Buffer,
        UAV
    } Type = EType::Unknown;

    ERHIResourceAccess AccessBefore = ERHIResourceAccess::Unknown;
    ERHIResourceAccess AccessAfter = ERHIResourceAccess::Unknown;

    // 范围信息：纹理、缓冲区、UAV 各自的子范围描述
    struct TextureRange
    {
        uint32_t MipLevel = 0;     // 起始mip级
        uint32_t MipCount = 1;     // mip级数量
        uint32_t ArraySlice = 0;   // 起始数组切片
        uint32_t ArraySize = 1;    // 切片数量
        uint32_t PlaneSlice = 0;   // 平面索引（若适用）
    } TextureRangeInfo;

    struct BufferRange
    {
        uint64_t Offset = 0;       // 字节偏移
        uint64_t Size = 0;         // 字节大小（0 表示整个缓冲区）
    } BufferRangeInfo;

    struct UAVRange
    {
        uint64_t FirstElement = 0; // 起始元素索引
        uint64_t NumElements = 0;  // 元素数量（0 表示到结尾或未指定）
    } UAVRangeInfo;

};
} //