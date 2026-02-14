#pragma once
#include "RHIResource.h"
#include <memory>
#include <string>

namespace RHI {

    // 类型区分
    enum class ERHITransientResourceType
    {
        Unknown,
        Texture,
        Buffer
    };

    // 内存分配类型（可扩展）
    enum class ERHITransientAllocationType
    {
        Heap,
        Page
    };

    // 简化 Transient Resource 基类
    class RHI_API RHITransientResource
    {
    public:
        virtual ~RHITransientResource();

        // 生命周期控制
        virtual void Acquire(const std::string& name, uint32_t passIndex);
        virtual void Release(uint32_t passIndex);

        bool IsAcquired() const;
        const std::string& GetName() const;
        ERHITransientResourceType GetResourceType() const;

    protected:
        explicit RHITransientResource(ERHITransientResourceType type);

    private:
        ERHITransientResourceType ResourceType = ERHITransientResourceType::Unknown;
        bool bAcquired = false;
        std::string Name;
        uint32_t AcquirePass = 0;
        uint32_t ReleasePass = 0;
    };

// TransientTexture

    class RHI_API RHITransientTexture : public RHITransientResource
    {
    public:
        explicit RHITransientTexture(const std::shared_ptr<RHITexture>& inTexture);

        std::shared_ptr<RHITexture> GetTexture() const;

    private:
        std::shared_ptr<RHITexture> Texture;
    };

    using RHITransientTextureSP = std::shared_ptr<RHITransientTexture>;
// TransientBuffer
    class RHI_API RHITransientBuffer : public RHITransientResource
    {
    public:
        explicit RHITransientBuffer(const std::shared_ptr<RHIBuffer>& inBuffer);
        std::shared_ptr<RHIBuffer> GetBuffer() const;

    private:
        std::shared_ptr<RHIBuffer> Buffer;
    };

    using RHITransientBufferSP = std::shared_ptr<RHITransientBuffer>;

    class RHI_API RHITransientResourceManager
    {
    public:
        virtual ~RHITransientResourceManager() = default;

        // 创建临时资源
        virtual RHITransientTextureSP CreateTransientTexture(const RHI::RHITextureDesc& desc) = 0;
        virtual RHITransientBufferSP CreateTransientBuffer(const RHI::RHIBufferDesc& desc) = 0;

        // 回收资源
        virtual void ReleaseTransientTexture(const RHITransientTextureSP& texture) = 0;
        virtual void ReleaseTransientBuffer(const RHITransientBufferSP& buffer) = 0;
    };

    using RHITransientResourceManagerSP = std::shared_ptr<RHITransientResourceManager>;

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