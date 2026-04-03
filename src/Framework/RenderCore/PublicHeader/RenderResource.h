#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include "RHITransientResource.h"
namespace RenderCore {

// 渲染资源生命周期管理基类
class RENDERCORE_API RenderResource
{
public:
    RenderResource();
    virtual ~RenderResource();

    virtual void InitRHIResource();
    virtual void ReleaseRHIResource();

    void SetName(const std::string& name);
    const std::string& GetName() const;

    bool IsInitialized() const;

protected:
    std::string Name;
    std::atomic<bool> bInitialized{ false };
};

// 纹理资源
class RENDERCORE_API RenderTexture : public RenderResource
{
public:
    RenderTexture();
    ~RenderTexture() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
    // 其他纹理相关接口
    RHI::RHITextureSP Texture;
};

// 通用缓冲区资源
class RENDERCORE_API RenderBuffer : public RenderResource
{
public:
    RenderBuffer();
    ~RenderBuffer() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
    // 其他缓冲区相关接口
};

// 顶点缓冲区
class RENDERCORE_API RenderVertexBuffer : public RenderBuffer
{
public:
    RenderVertexBuffer();
    ~RenderVertexBuffer() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
    // 其他顶点缓冲区相关接口
};

// 索引缓冲区
class RENDERCORE_API RenderIndexBuffer : public RenderBuffer
{
public:
    RenderIndexBuffer();
    ~RenderIndexBuffer() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
    // 其他索引缓冲区相关接口
};


struct RENDERCORE_API PoolRenderTargetDesc
{

    // ----------------------
    // 1. 尺寸 / 形态
    // ----------------------
    uint32_t Width = 1;
    uint32_t Height = 1;
    uint32_t Depth = 1;

    uint32_t ArraySize = 1;
    uint32_t MipLevels = 1;
    uint32_t SampleCount = 1;

    RHI::ERHITextureType Type = RHI::ERHITextureType::Texture2D;

    // ----------------------
    // 2. 格式
    // ----------------------
    RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;

    // UAV 可能用不同格式（可选）
    RHI::ERHIFormat UAVFormat = RHI::ERHIFormat::Unknown;

    // ----------------------
    // 3. 用途（ImageUsage / BindFlags）
    // ----------------------
    RHI::ERHITextureCreateFlags Usage = RHI::ERHITextureCreateFlag::None;
   
    // ----------------------
    // 4. Clear（RHI 级）
    // ----------------------
    RHI::RHIClearValueBinding ClearValue;

    // ----------------------
    // 5. Debug
    // ----------------------
    const char* DebugName = "UnnamedPoolRT";

    // ----------------------
    // 6. 语义函数
    // ----------------------
    bool IsValid() const
    {
        return Width > 0
            && Height > 0
            && MipLevels > 0
            && SampleCount > 0
            && Format != RHI::ERHIFormat::Unknown;
    }

    bool IsDepthStencil() const
    {
        return ClearValue.Binding == RHI::RHIClearValueBinding::ClearValueBinding::DepthStencil;
    }

    bool IsColor() const
    {
        return ClearValue.Binding == RHI::RHIClearValueBinding::ClearValueBinding::Color;
    }

    static PoolRenderTargetDesc ConvertFromRHITextureDesc(const RHI::RHITextureDesc& InDesc)
    {
        PoolRenderTargetDesc Desc;

        Desc.Width = static_cast<int32_t>(InDesc.Width);
        Desc.Height =static_cast<int32_t>(InDesc.Height) ;
        Desc.Depth = static_cast<uint16_t>(InDesc.Depth);
        Desc.ArraySize = static_cast<uint16_t>(InDesc.ArraySize);
        Desc.MipLevels = static_cast<uint8_t>(InDesc.MipLevels);
        Desc.SampleCount = static_cast<uint8_t>(InDesc.SampleCount);
        Desc.Format = InDesc.Format;  // 假设 ERHIFormat 与 EPixelFormat 可映射
        Desc.Usage = InDesc.Usage;

        Desc.DebugName = InDesc.DebugName ? InDesc.DebugName : "RT_From_RHITextureDesc";

        return Desc;
    }

    static RHI::RHITextureDesc ConvertToRHITextureDesc(const PoolRenderTargetDesc& InDesc) {
		RHI::RHITextureDesc Desc;
		Desc.Width = InDesc.Width;
		Desc.Height = InDesc.Height;
		Desc.Depth = InDesc.Depth;
		Desc.ArraySize = InDesc.ArraySize;
		Desc.MipLevels = InDesc.MipLevels;
		Desc.SampleCount = InDesc.SampleCount;
		Desc.Format = InDesc.Format;
		Desc.Type = InDesc.Type;
		Desc.Usage = InDesc.Usage;
		Desc.DebugName = InDesc.DebugName;
		return Desc;
    }

};

struct RENDERCORE_API IPooledRenderTarget
{
    virtual ~IPooledRenderTarget() = default;

    // ------------------------
    // Description
    // ------------------------
    virtual const PoolRenderTargetDesc& GetDesc() const = 0;

    // ------------------------
    // RHI Access
    // ------------------------
    virtual RHI::RHITexture* GetRHI() { return nullptr; };

	virtual RHI::RHITransientTexture* GetTransientRHI() { return nullptr; };
protected:



   
};


class RENDERCORE_API PooledRenderTarget final : public IPooledRenderTarget
{
public:
    PooledRenderTarget(
        const PoolRenderTargetDesc& InDesc,
        RHI::RHITextureSP InTexture)
        : Desc(InDesc)
    {
        TargetTexture = InTexture;
    }

    virtual const PoolRenderTargetDesc& GetDesc() const override
    {
        return Desc;
    }


private:
    PoolRenderTargetDesc Desc;
    RHI::RHITextureSP TargetTexture;
};

class RENDERCORE_API PooledTransientRenderTarget final : public IPooledRenderTarget
{
public:
    PooledTransientRenderTarget(
        const PoolRenderTargetDesc& InDesc,
        RHI::RHITransientTexture* InTransientTexture)
        : Desc(InDesc)
        , TransientTexture(InTransientTexture)
    {

    }

    virtual const PoolRenderTargetDesc& GetDesc() const override
    {
        return Desc;
    }

    virtual RHI::RHITransientTexture* GetTransientRHI() override
    {
        return TransientTexture;
    }

    // 可选：如果你希望某些地方仍然能拿到“底层纹理”
    virtual RHI::RHITexture* GetRHI() override
    {
        return TransientTexture
            ? TransientTexture->GetTexture().get()
            : nullptr;
    }

private:
    PoolRenderTargetDesc Desc;
    RHI::RHITransientTexture* TransientTexture = nullptr;
};

// -------------------------------
// 精简 RenderTargetPool
// -------------------------------
class RENDERCORE_API RenderTargetPool
{
public:
    RenderTargetPool() = default;

    // 分配或复用 RenderTarget
    std::shared_ptr<PooledRenderTarget> AllocateRenderTarget(
        const PoolRenderTargetDesc& Desc);

    // 回收 RenderTarget
    void Release(std::shared_ptr<PooledRenderTarget> RenderTarget);

    // 可选：清空池
    void Clear();

private:
    std::mutex Mutex;
    std::vector<std::shared_ptr<PooledRenderTarget>> FreeList;
};

extern RENDERCORE_API RenderTargetPool* GRenderTargetPool;

// -------------------------------
// 精简 Transient Allocator（无 PassHandle）
// -------------------------------
class RENDERCORE_API TransientResourceAllocator : public RenderResource
{
public:
    TransientResourceAllocator() = default;
    virtual void InitRHIResource() override {}
    virtual void ReleaseRHIResource() override {}

    // 分配 transient render target
    std::shared_ptr<PooledTransientRenderTarget> AllocateRenderTarget(
        const PoolRenderTargetDesc& Desc,
        RHI::RHITransientTexture* Texture)
    {
        std::lock_guard<std::mutex> Lock(Mutex);

        if (!FreeList.empty())
        {
            auto RT = FreeList.back();
            FreeList.pop_back();
            return RT;
        }

        // 创建新的
        auto RT = std::make_shared<PooledTransientRenderTarget>(Desc, Texture);
        return RT;
    }

    // 释放 render target（立即放回 free list）
    void Release(std::shared_ptr<PooledTransientRenderTarget> RenderTarget)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        FreeList.push_back(RenderTarget);
    }

private:
    RHI::RHITransientResourceManagerSP Allocator = nullptr;
    std::mutex Mutex;
    std::vector<std::shared_ptr<PooledTransientRenderTarget>> FreeList;
};

extern RENDERCORE_API TransientResourceAllocator* GTransientResourceAllocator;

// 智能指针类型
using RenderResourceSP = std::shared_ptr<RenderResource>;
using RenderTextureSP = std::shared_ptr<RenderTexture>;
using RenderBufferSP = std::shared_ptr<RenderBuffer>;
using RenderVertexBufferSP = std::shared_ptr<RenderVertexBuffer>;
using RenderIndexBufferSP = std::shared_ptr<RenderIndexBuffer>;

extern RENDERCORE_API RenderTexture* GlobalTestTexture;

RENDERCORE_API bool InitGlobalRenderResource();
RENDERCORE_API void ReleaseGlobalRenderResource();

RENDERCORE_API RenderTexture* CreateTexture(const std::string& Path);

} // namespace RenderCore