#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include "RHITransientResource.h"
#include "RHIDefine.h"
#include "RHICommandContex.h"
#include "RenderResourceTracker.h"
#include "RHIApi.h"

namespace RenderCore {

    class TextureViewCache
    {
    public:
        TextureViewCache()
        {
        }

        RHI::RHIShaderResourceView* GetOrCreateSRV(
            RHI::RHITexture* texture,
            const RHI::RHITexSRVCreateInfo& desc)
        {
            auto it = SRVs.find(desc);
            if (it != SRVs.end())
                return it->second.get
                ();

            auto view = RHI::GRHIApi->CreateTextureShaderResourceView(texture, desc);
            SRVs.emplace(desc, view);
            return view.get();
        }

        RHI::RHIUnorderedAccessView* GetOrCreateUAV(
            RHI::RHITexture* texture,
            const RHI::RHITexUAVCreateInfo& desc)
        {
            auto it = UAVs.find(desc);
            if (it != UAVs.end())
                return it->second.get();

            auto view = RHI::GRHIApi->CreateTextureUnorderedAccessView(texture, desc);
            UAVs.emplace(desc, view);
            return view.get();
        }

        void Clear()
        {
            SRVs.clear();
            UAVs.clear();
        }

    private:
        std::unordered_map<RHI::RHITexSRVCreateInfo, RHI::RHIShaderResourceViewSP> SRVs;
        std::unordered_map<RHI::RHITexUAVCreateInfo, RHI::RHIUnorderedAccessViewSP> UAVs;
    };


    class BufferViewCache
    {
    public:
        BufferViewCache()
        {
        }

        RHI::RHIShaderResourceView* GetOrCreateSRV(
            RHI::RHIBuffer* buffer,
            const RHI::RHIBufferSRVCreateInfo& desc)
        {
            auto it = SRVs.find(desc);
            if (it != SRVs.end())
                return it->second.get();

            auto view = RHI::GRHIApi->CreateBufferShaderResourceView(buffer, desc);
            SRVs.emplace(desc, view);
            return view.get();
        }

        RHI::RHIUnorderedAccessView* GetOrCreateUAV(
            RHI::RHIBuffer* buffer,
            const RHI::RHIBufferUAVCreateInfo& desc)
        {
            auto it = UAVs.find(desc);
            if (it != UAVs.end())
                return it->second.get();

            auto view = RHI::GRHIApi->CreateBufferUnorderedAccessView(buffer, desc);
            UAVs.emplace(desc, view);
            return view.get();
        }

        void Clear()
        {
            SRVs.clear();
            UAVs.clear();
        }

    private:
        std::unordered_map<RHI::RHIBufferSRVCreateInfo, RHI::RHIShaderResourceViewSP> SRVs;
        std::unordered_map<RHI::RHIBufferUAVCreateInfo, RHI::RHIUnorderedAccessViewSP> UAVs;
    };

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
    void UploadData(const void* data, uint32_t mipIndex ,uint32_t arraySlice,uint32_t planeSlice = 0);
    RHI::RHITexture* GetRHI() const { return Texture.get(); }
    RenderTextureTracker& GetTracker() { return Tracker; }
    TextureViewCache& GetViewCache() { return ViewCache; }
private:
    TextureViewCache ViewCache;
    RenderTextureTracker Tracker;
    // 其他纹理相关接口
	RHI::RHITextureDesc Desc;
    RHI::RHITextureSP Texture;
};


RENDERCORE_API void TransitionTextureImmediate(
    RHI::RHIApi* api,
    RenderTexture* resource,
    const RHI::RHISubresourceRange& range,
    RHI::ERHIResourceAccess targetAccess,
    RHI::EQueueType targetQueueType);

// 通用缓冲区资源
class RENDERCORE_API RenderBuffer : public RenderResource
{
public:
    RenderBuffer(const RHI::RHIBufferDesc& inDesc);
    ~RenderBuffer() override;

    void InitRHIResource() override;
    void ReleaseRHIResource() override;
	void UploadData(const void* data, uint32_t size, uint32_t offset = 0);
    // 其他缓冲区相关接口
	RHI::RHIBuffer* GetRHI() const { return Buffer.get(); }
    RenderBufferTracker& GetTracker() { return Tracker; }
	BufferViewCache& GetViewCache() { return ViewCache; }
private:
    BufferViewCache ViewCache;
    RenderBufferTracker Tracker;
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
    bool Matches(const PoolRenderTargetDesc& rhs) const
    {
        return
            Type == rhs.Type &&
            Format == rhs.Format &&
            SampleCount == rhs.SampleCount &&

            IsDepthStencil() == rhs.IsDepthStencil() &&
            IsColor() == rhs.IsColor() &&

            (Usage & rhs.Usage) == rhs.Usage &&

            Width >= rhs.Width &&
            Height >= rhs.Height &&
            Depth >= rhs.Depth &&

            MipLevels >= rhs.MipLevels &&
            ArraySize >= rhs.ArraySize;
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
    TextureViewCache& GetViewCache() { return ViewCache; }
protected:
    TextureViewCache ViewCache;
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
    RHI::RHITexture* GetRHI() override { return TargetTexture ? TargetTexture.get() : nullptr ; } ;

    void MarkUsed(bool InIsUsed = true) { IsUse = InIsUsed; }
    bool IsUsed() const { return IsUse; }
private:
    bool IsUse = false;
    PoolRenderTargetDesc Desc;
    RHI::RHITextureSP TargetTexture;
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

extern RENDERCORE_API RenderTargetPool GRenderTargetPool;

struct TransientBufferDesc {
    uint64_t Size = 0;                   // 缓冲区大小（字节）
    uint32_t Stride = 0;                 // 结构化缓冲区的元素大小
    RHI::ERHIBufferUsageFlags Usage = RHI::ERHIBufferUsageFlag::None; // 缓冲区用途
    bool bCPUAccessible = false;         // CPU是否可访问
    RHI::EQueueType InitialQueueType = RHI::EQueueType::Graphics; // 初始所属队列
    static RHI::RHIBufferDesc ConvertToRHIBufferDesc(const TransientBufferDesc& InDesc) {
        RHI::RHIBufferDesc Desc;
        Desc.Size = InDesc.Size;
        Desc.Stride = InDesc.Stride;
        Desc.Usage = InDesc.Usage;
        Desc.bCPUAccessible = InDesc.bCPUAccessible;
        Desc.InitialQueueType = InDesc.InitialQueueType;
        return Desc;
    }
    static TransientBufferDesc ConvertFromRHIBufferDesc(const RHI::RHIBufferDesc& InDesc) {
        TransientBufferDesc Desc;
        Desc.Size = InDesc.Size;
        Desc.Stride = InDesc.Stride;
        Desc.Usage = InDesc.Usage;
        Desc.bCPUAccessible = InDesc.bCPUAccessible;
        Desc.InitialQueueType = InDesc.InitialQueueType;
        return Desc;
    }
};

class PooledTransientBuffer
{
public:
	PooledTransientBuffer(
		const TransientBufferDesc& InDesc,
		RHI::RHITransientBufferSP InTransientBuffer)
		: Desc(InDesc)
		, TransientBuffer(std::move(InTransientBuffer))
	{
	}
    ~PooledTransientBuffer()
    {
        ViewCache.Clear();
        TransientBuffer.reset();
    }
    const TransientBufferDesc& GetDesc() const { return Desc; }
    RHI::RHIBuffer* GetRHI() const { return TransientBuffer->GetBuffer().get(); }
    RHI::RHITransientBuffer* GetTransientRHI() const { return TransientBuffer.get(); }
    RenderBufferTracker& GetTracker() { return Tracker; }
    BufferViewCache& GetViewCache() { return ViewCache; }
private:
    friend class TransientResourceAllocator;
    BufferViewCache ViewCache;
    RenderBufferTracker Tracker;
    TransientBufferDesc Desc;
    RHI::RHITransientBufferSP TransientBuffer = nullptr;


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
    ~PooledTransientRenderTarget()
    {
        ViewCache.Clear();
        TransientTexture.reset();
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
    friend class TransientResourceAllocator;
    PoolRenderTargetDesc Desc;
    // ✅ 改成 shared_ptr（核心）
    RHI::RHITransientTextureSP TransientTexture;
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
    ~TransientResourceAllocator() = default;
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
        auto target = std::make_shared<PooledTransientRenderTarget>(
            Desc,
            transientTex);
        AllocatedTextures.push_back(target);
        return target;
    }
    std::shared_ptr<PooledTransientBuffer> AllocateBuffer(
        const TransientBufferDesc& Desc,
        uint32_t beginIndex,
        uint32_t endIndex)
    {
        // 👉 直接创建 transient texture（核心）
        auto transientBuf = TransientResourceManager->CreateTransientBuffer(
            TransientBufferDesc::ConvertToRHIBufferDesc(Desc),
            beginIndex,
            endIndex);

        // 👉 创建 wrapper（不复用）
        auto buf = std::make_shared<PooledTransientBuffer>(
            Desc,
            transientBuf);
        AllocatedBuffers.push_back(buf);
        return buf;
    }

    void GarbageCollect();
private:
    
    std::vector<std::shared_ptr<PooledTransientRenderTarget>> AllocatedTextures;
    std::vector<std::shared_ptr<PooledTransientBuffer>>      AllocatedBuffers;
    RHI::RHITransientResourceManagerSP TransientResourceManager;
};

extern RENDERCORE_API TransientResourceAllocator GTransientResourceAllocator;

// 智能指针类型
using RenderResourceSP = std::shared_ptr<RenderResource>;
using RenderTextureSP = std::shared_ptr<RenderTexture>;
using RenderBufferSP = std::shared_ptr<RenderBuffer>;

extern RENDERCORE_API RenderTextureSP GlobalTestTexture;
extern RENDERCORE_API RHI::RHISamplerSP GlobalSampler;
RENDERCORE_API bool InitGlobalRenderResource();
RENDERCORE_API void ReleaseGlobalRenderResource();

RENDERCORE_API RenderTextureSP CreateTexture(const std::string& Path);

} // namespace RenderCore