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

        AllocateDedicatedTexture(
            Resolution,
            Resolution,
            EShadowType::Directional,
            result);
        return result;
    }

    ShadowAllocation ShadowMapAllocator::AllocatePointShadow(
        uint32_t Resolution)
    {
        ShadowAllocation result;
        result.ShadowType = EShadowType::Point;

        result.Slices.resize(MaxPointLightFaces);

        AllocateDedicatedTexture(
            Resolution,
            Resolution,
            EShadowType::Point,
            result);

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
            result);

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
            // Dedicated shadow texture
            //--------------------------------------------------
        case EShadowType::Directional:
        case EShadowType::Point:
        {
            auto* texture = Allocation.Texture;

            if (texture)
            {
                auto it = std::find_if(
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

                //--------------------------------------------------
                // Remove from lookup array
                //--------------------------------------------------

                auto eraseTexture =
                    [texture](std::vector<RenderCore::RenderTexture*>& list)
                    {
                        auto it =
                            std::find(list.begin(), list.end(), texture);

                        if (it != list.end())
                        {
                            list.erase(it);
                        }
                    };

                eraseTexture(PointLightShadowTextures_);
                eraseTexture(ParallelLightShadowTextures_);
            }

            break;
        }

        //--------------------------------------------------
        // Atlas
        //--------------------------------------------------
        case EShadowType::Spot:
            // 后续实现 Atlas FreeList
            break;

        default:
            break;
        }

        Allocation.Texture = nullptr;
        Allocation.TextureIndex = 0;
        Allocation.Slices.clear();
        Allocation.ShadowType = EShadowType::Unknown;
    }

    bool ShadowMapAllocator::AllocateDedicatedTexture(
        uint32_t Width,
        uint32_t Height,
        EShadowType ShadowType,
        ShadowAllocation& OutAllocation)
    {
        RHITextureDesc desc;

        desc.Width = Width;
        desc.Height = Height;
        desc.Depth = 1;

        desc.MipLevels = 1;
        desc.ArraySize = 1;

        desc.Type = ERHITextureType::Texture2D;
        switch (ShadowType)
        {
        case EShadowType::Directional:
            desc.Type = ERHITextureType::Texture2DArray;
            desc.ArraySize = MaxShadowCascadeCount;
            desc.DebugName = "DirectionalShadow";
            break;

        case EShadowType::Point:
            desc.Type = ERHITextureType::TextureCube;
            desc.ArraySize = 6;
            desc.DebugName = "PointShadow";
            break;

        default:
            return false;
        }
        desc.Format = ERHIFormat::R16G16_Float;
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
        OutAllocation.ShadowType = ShadowType;

        switch (ShadowType)
        {
        case EShadowType::Point:
        {
            OutAllocation.TextureIndex =
                (uint32_t)PointLightShadowTextures_.size();

            PointLightShadowTextures_.push_back(texture.get());
            break;
        }

        case EShadowType::Directional:
        {
            OutAllocation.TextureIndex =
                (uint32_t)ParallelLightShadowTextures_.size();

            ParallelLightShadowTextures_.push_back(texture.get());
            break;
        }

        default:
            OutAllocation.TextureIndex = 0;
            break;
        }
        //-----------------------------------------
        // Fill slices
        //-----------------------------------------

        OutAllocation.Slices.resize(desc.ArraySize);

        for (uint32_t i = 0; i < desc.ArraySize; ++i)
        {
            auto& slice = OutAllocation.Slices[i];

            slice.Layer = i;
            slice.Mip = 0;

            slice.X = 0;
            slice.Y = 0;

            slice.Width = Width;
            slice.Height = Height;

            slice.UVScale =
            {
                1.0f,
                1.0f
            };

            slice.UVOffset =
            {
                0.0f,
                0.0f
            };
        }

        return true;
    }

    bool ShadowMapAllocator::AllocateFromAtlas(
        uint32_t Width,
        uint32_t Height,
        ShadowAllocation& OutAllocation)
    {
        const uint32_t AtlasWidth = Desc_.SpotShadowAtlas.Width;
        const uint32_t AtlasHeight = Desc_.SpotShadowAtlas.Height;

        //--------------------------------------------------
        // 当前行放不下，换行
        //--------------------------------------------------

        if (AtlasCursorX + Width > AtlasWidth)
        {
            AtlasCursorX = 0;
            AtlasCursorY += AtlasCurrentRowHeight;
            AtlasCurrentRowHeight = 0;
        }

        //--------------------------------------------------
        // Atlas 已满
        //--------------------------------------------------

        if (AtlasCursorY + Height > AtlasHeight)
        {
            return false;
        }

        //--------------------------------------------------
        // 创建 Allocation
        //--------------------------------------------------

        OutAllocation.ShadowType = EShadowType::Spot;
        OutAllocation.Texture = ShadowAtlas_.get();
        OutAllocation.TextureIndex = 0;

        OutAllocation.Slices.clear();
        OutAllocation.Slices.emplace_back();

        auto& Slice = OutAllocation.Slices.front();

        Slice.Layer = 0;
        Slice.Mip = 0;

        Slice.X = AtlasCursorX;
        Slice.Y = AtlasCursorY;

        Slice.Width = Width;
        Slice.Height = Height;

        Slice.UVScale =
        {
            float(Width) / float(AtlasWidth),
            float(Height) / float(AtlasHeight)
        };

        Slice.UVOffset =
        {
            float(AtlasCursorX) / float(AtlasWidth),
            float(AtlasCursorY) / float(AtlasHeight)
        };

        //--------------------------------------------------
        // 更新游标
        //--------------------------------------------------

        AtlasCursorX += Width;

        AtlasCurrentRowHeight =
            AtlasCurrentRowHeight > Height ? AtlasCurrentRowHeight : Height;

        return true;
    }
}