#include "ShadowMapAllocator.h"
using namespace RHI;
using namespace RenderCore;
namespace Renderer
{

    ShadowMapAllocator::ShadowMapAllocator()
    {
    }

    ShadowMapAllocator::~ShadowMapAllocator()
    {
        Shutdown();
    }

    bool ShadowMapAllocator::Initialize(
        const ShadowAllocatorDesc& Desc)
    {
        Desc_ = Desc;

        return InitializeAtlasResources();
    }

    const ShadowAllocatorDesc& ShadowMapAllocator::GetDesc() const
    {
        return Desc_;
    }

    void ShadowMapAllocator::Shutdown()
    {
        ShadowAtlas_ = nullptr;
        DedicatedShadowTextures_.clear();
    }

    bool ShadowMapAllocator::InitializeAtlasResources()
    {
        RHITextureDesc desc;
        desc.Width = Desc_.SpotShadowAtlas.Width;
        desc.Height = Desc_.SpotShadowAtlas.Height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.ArraySize = 1;

        desc.Format = ERHIFormat::R16G16_Float;
        desc.Type = ERHITextureType::Texture2D;
        desc.SampleCount = 1;

        desc.Usage =
            ERHITextureCreateFlag::RenderTarget |
            ERHITextureCreateFlag::ShaderResource;

        desc.bGenerateMips = false;
        desc.DebugName = "SpotShadowAtlas";

        ShadowAtlas_ =
            std::make_shared<RenderCore::RenderTexture>(desc);

        ShadowAtlas_->InitRHIResource();

        return ShadowAtlas_ != nullptr;
    }

    ShadowAllocation ShadowMapAllocator::AllocateDirectionalCSMShadow(
        uint32_t Resolution,
        uint32_t CascadeCount)
    {
        ShadowAllocation result;
        result.ShadowType = EShadowType::Directional;

        result.Slices.resize(CascadeCount);

        for (uint32_t i = 0; i < CascadeCount; i++)
        {
            AllocateDedicatedTexture(
                Resolution,
                Resolution,
                EShadowType::Directional,
                result.Slices[i]);
        }

        return result;
    }

    ShadowAllocation ShadowMapAllocator::AllocatePointShadow(
        uint32_t Resolution)
    {
        ShadowAllocation result;
        result.ShadowType = EShadowType::Point;

        result.Slices.resize(MaxPointLightFaces);

        for (uint32_t i = 0; i < MaxPointLightFaces; i++)
        {
            AllocateDedicatedTexture(
                Resolution,
                Resolution,
                EShadowType::Point,
                result.Slices[i]);
        }

        return result;
    }

    ShadowAllocation ShadowMapAllocator::AllocateSpotShadow(
        uint32_t Resolution)
    {
        ShadowAllocation result;
        result.ShadowType = EShadowType::Spot;

        result.Slices.resize(1);

        AllocateFromAtlas(
            Resolution,
            Resolution,
            result.Slices[0]);

        return result;
    }

    RenderCore::RenderTexture* ShadowMapAllocator::GetShadowAtlas() const
    {
        return ShadowAtlas_.get();
    }

    const std::vector<RenderCore::RenderTexture*>& ShadowMapAllocator::GetDedicatedPointLightShadowTextures() const
    {
        return PointLightShadowTextures_;
    }

    const std::vector<RenderCore::RenderTexture*>& ShadowMapAllocator::GetDedicatedParallelLightShadowTextures() const
    {
        return ParallelLightShadowTextures_;
    }

    void ShadowMapAllocator::ReleaseShadow(ShadowAllocation& Allocation)
    {
        if (!Allocation.IsValid())
        {
            return;
        }

        switch (Allocation.ShadowType)
        {
            //--------------------------------------------------
            // Dedicated texture shadow
            //--------------------------------------------------
        case EShadowType::Directional:
        case EShadowType::Point:
        {
            for (auto& slice : Allocation.Slices)
            {
                auto* texture =
                    slice.Texture;

                if (!texture)
                    continue;

                auto it =
                    std::find_if(
                        DedicatedShadowTextures_.begin(),
                        DedicatedShadowTextures_.end(),
                        [texture](const RenderCore::RenderTextureSP& tex)
                        {
                            return tex.get() == texture;
                        });

                if (it != DedicatedShadowTextures_.end())
                {
                    DedicatedShadowTextures_.erase(it);
                }

                slice.Texture = nullptr;
            }

            break;
        }

        //--------------------------------------------------
        // Atlas shadow
        //--------------------------------------------------
        case EShadowType::Spot:
        {
            // 当前版本先不回收 atlas 区域
            // 后面如果实现 free list / buddy allocator
            // 再把 region 放回 allocator
            for (auto& slice : Allocation.Slices)
            {
                slice.Texture = nullptr;
            }

            break;
        }

        default:
            break;
        }

        Allocation.Slices.clear();
        Allocation.ShadowType = EShadowType::Unknown;
    }

    bool ShadowMapAllocator::AllocateDedicatedTexture(
        uint32_t Width,
        uint32_t Height,
        EShadowType ShadowType,
        ShadowAllocationSlice& OutAllocation)
    {
        RHITextureDesc desc;
        desc.Width = Width;
        desc.Height = Height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.ArraySize = 1;

        desc.Format = ERHIFormat::R16G16_Float;
        desc.Type = ERHITextureType::Texture2D;
        desc.SampleCount = 1;

        desc.Usage =
            ERHITextureCreateFlag::RenderTarget |
            ERHITextureCreateFlag::ShaderResource;

        desc.bGenerateMips = false;
        desc.DebugName = "ShadowMap";

        auto texture =
            std::make_shared<RenderCore::RenderTexture>(desc);

        texture->InitRHIResource();

        DedicatedShadowTextures_.push_back(texture);

        OutAllocation.Texture = texture.get();

        OutAllocation.Layer = 0;
        OutAllocation.Mip = 0;

        OutAllocation.X = 0;
        OutAllocation.Y = 0;

        OutAllocation.Width = Width;
        OutAllocation.Height = Height;

        OutAllocation.UVScale = { 1.0f, 1.0f };
        OutAllocation.UVOffset = { 0.0f, 0.0f };

        return true;
    }

    bool ShadowMapAllocator::AllocateFromAtlas(
        uint32_t Width,
        uint32_t Height,
        ShadowAllocationSlice& OutAllocation)
    {
        static uint32_t CurrentX = 0;
        static uint32_t CurrentY = 0;

        const uint32_t AtlasWidth = Desc_.SpotShadowAtlas.Width;
        const uint32_t AtlasHeight = Desc_.SpotShadowAtlas.Height;

        if (CurrentX + Width > AtlasWidth)
        {
            CurrentX = 0;
            CurrentY += Height;
        }

        if (CurrentY + Height > AtlasHeight)
        {
            return false;
        }

        OutAllocation.Texture = ShadowAtlas_.get();

        OutAllocation.Layer = 0;
        OutAllocation.Mip = 0;

        OutAllocation.X = CurrentX;
        OutAllocation.Y = CurrentY;
        OutAllocation.Width = Width;
        OutAllocation.Height = Height;

        OutAllocation.UVScale =
        {
            (float)Width / AtlasWidth,
            (float)Height / AtlasHeight
        };

        OutAllocation.UVOffset =
        {
            (float)CurrentX / AtlasWidth,
            (float)CurrentY / AtlasHeight
        };

        CurrentX += Width;

        return true;
    }
}