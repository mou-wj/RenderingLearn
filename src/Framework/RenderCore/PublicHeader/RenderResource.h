#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include "RHITransientResource.h"
#include "RHIDefine.h"
#include "RHICommandContex.h"
#include "RenderResourceTracker.h"
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
    RenderTexture(const RHI::RHITextureDesc& inDesc);
    ~RenderTexture() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
    RHI::RHITextureSP GetRHI() const { return Texture; }
private:
    // 其他纹理相关接口
	RHI::RHITextureDesc Desc;
    RHI::RHITextureSP Texture;
};

// 通用缓冲区资源
class RENDERCORE_API RenderBuffer : public RenderResource
{
public:
    RenderBuffer(const RHI::RHIBufferDesc& inDesc);
    ~RenderBuffer() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
    // 其他缓冲区相关接口
	RHI::RHIBufferSP GetRHI() const { return Buffer; }
private:
    RHI::RHIBufferDesc Desc;
    RHI::RHIBufferSP Buffer;
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

    RenderTextureTracker& GetTracker() { return Tracker; }
protected:
    RenderTextureTracker Tracker;
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
        RHI::RHITransientTextureSP InTransientTexture)
        : Desc(InDesc)
        , TransientTexture(std::move(InTransientTexture))
    {
    }

    virtual const PoolRenderTargetDesc& GetDesc() const override
    {
        return Desc;
    }

    virtual RHI::RHITransientTexture* GetTransientRHI()
    {
        return TransientTexture.get();
    }

    virtual RHI::RHITexture* GetRHI() override
    {
        return TransientTexture
            ? TransientTexture->GetTexture().get()
            : nullptr;
    }

private:
    PoolRenderTargetDesc Desc;

    // ✅ 改成 shared_ptr（核心）
    RHI::RHITransientTextureSP TransientTexture;
};

// -------------------------------
// 精简 RenderTargetPool
// -------------------------------

class RENDERCORE_API RenderTargetPool
{
public:
    RenderTargetPool() = default;

    // 分配或复用 RenderTarget（重命名）
    std::shared_ptr<PooledRenderTarget> GetFreeRenderTarget(
        const PoolRenderTargetDesc& Desc);

    // 可选：清空池
    void Clear();

private:
    // 垃圾回收：检查所有已分配target，若frame小于当前queue的frame则回收
    void GarbageCollect();

    std::mutex Mutex;
    std::vector<std::shared_ptr<PooledRenderTarget>> FreeList;
    // 记录所有已分配的target，便于垃圾回收
    std::vector<std::shared_ptr<PooledRenderTarget>> AllocatedList;
};

extern RENDERCORE_API RenderTargetPool* GRenderTargetPool;

struct TransientBufferDesc{

};

class PooledTransientBuffer
{
public:
	PooledTransientBuffer(
		const RHI::RHIBufferDesc& InDesc,
		RHI::RHITransientBufferSP InTransientBuffer)
		: Desc(InDesc)
		, TransientBuffer(std::move(InTransientBuffer))
	{
	}

    const RHI::RHIBufferDesc& GetDesc() const { return Desc; }
    RHI::RHIBuffer* GetRHI() const { return TransientBuffer->GetBuffer().get(); }
private:
    RHI::RHIBufferDesc Desc;
    RHI::RHITransientBufferSP TransientBuffer = nullptr;


};

// -------------------------------
// 精简 Transient Allocator（无 PassHandle）
// -------------------------------
// --------------------------------------
// TransientResourceAllocator（优化版）
// --------------------------------------
class RENDERCORE_API TransientResourceAllocator
{
public:
    TransientResourceAllocator() = default;
    void InitRHI();
    void ReleaseRHI();

    // -------------------------------
    // Allocate
    // -------------------------------
    std::shared_ptr<PooledTransientRenderTarget> AllocateRenderTarget(
        const PoolRenderTargetDesc& Desc,
        uint32_t beginIndex,
        uint32_t endIndex)
    {
        // 👉 直接创建 transient texture（核心）
        auto transientTex = TransientResourceManager->CreateTransientTexture(
            PoolRenderTargetDesc::ConvertToRHITextureDesc(Desc),
            beginIndex,
            endIndex);

        // 👉 创建 wrapper（不复用）
        return std::make_shared<PooledTransientRenderTarget>(
            Desc,
            transientTex);
    }
    std::shared_ptr<PooledTransientBuffer> AllocateBuffer(
        const RHI::RHIBufferDesc& Desc,
        uint32_t beginIndex,
        uint32_t endIndex)
    {
        // 👉 直接创建 transient texture（核心）
        auto transientTex = TransientResourceManager->CreateTransientBuffer(
            Desc,
            beginIndex,
            endIndex);

        // 👉 创建 wrapper（不复用）
        return std::make_shared<PooledTransientBuffer>(
            Desc,
            transientTex);
    }


private:

    RHI::RHITransientResourceManagerSP TransientResourceManager;
};

extern RENDERCORE_API TransientResourceAllocator* GTransientResourceAllocator;

// 智能指针类型
using RenderResourceSP = std::shared_ptr<RenderResource>;
using RenderTextureSP = std::shared_ptr<RenderTexture>;
using RenderBufferSP = std::shared_ptr<RenderBuffer>;

extern RENDERCORE_API RenderTexture* GlobalTestTexture;

RENDERCORE_API bool InitGlobalRenderResource();
RENDERCORE_API void ReleaseGlobalRenderResource();

RENDERCORE_API RenderTexture* CreateTexture(const std::string& Path);

} // namespace RenderCore