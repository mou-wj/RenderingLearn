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
        virtual void Acquire(const std::string& name, uint32_t beginIndex);
        virtual void Release(uint32_t endIndex);

        bool IsAcquired() const;
        const std::string& GetName() const;
        ERHITransientResourceType GetResourceType() const;

    protected:
        explicit RHITransientResource(ERHITransientResourceType type);

    private:
        ERHITransientResourceType ResourceType = ERHITransientResourceType::Unknown;
        bool bAcquired = false;
        std::string Name;
        uint32_t BeginIndex = 0;
        uint32_t EndIndex = 0;
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
        virtual RHITransientTextureSP CreateTransientTexture(const RHI::RHITextureDesc& desc, uint32_t beginIndex,
            uint32_t endIndex) = 0;
        virtual RHITransientBufferSP CreateTransientBuffer(const RHI::RHIBufferDesc& desc, uint32_t beginIndex,
            uint32_t endIndex) = 0;

        // 回收资源
        virtual void ReleaseTransientTexture(const RHITransientTexture* texture, uint32_t endIndex) = 0;
        virtual void ReleaseTransientBuffer(const RHITransientBuffer* buffer, uint32_t endIndex) = 0;
    };

    using RHITransientResourceManagerSP = std::shared_ptr<RHITransientResourceManager>;

} //