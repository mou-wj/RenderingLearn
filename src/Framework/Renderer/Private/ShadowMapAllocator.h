#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cstdint>
#include "RenderResource.h"
#include "Math.h"
#include "SceneShaderParameters.h"
namespace Renderer
{
    class Scene;
    class LightComponent;

    static constexpr uint32_t MaxShadowCascadeCount = 4;
    static constexpr uint32_t MaxPointLightFaces = 6;

    /*
    ============================================================
        Shadow Type
    ============================================================
    */
    enum class EShadowType : uint8_t
    {
        Unknown,

        Directional,
        Spot,
        Point
    };

    /*
    ============================================================
        Shadow Slice
        Allocation + Matrix
    ============================================================
    */
    struct ShadowAllocationSlice
    {
        

        uint32_t Layer = 0;
        uint32_t Mip = 0;

        /*
        ------------------------------------------------------------
            Render viewport in texture
        ------------------------------------------------------------
        */
        uint32_t X = 0;
        uint32_t Y = 0;
        uint32_t Width = 0;
        uint32_t Height = 0;

        /*
        ------------------------------------------------------------
            Sampling UV transform
            finalUV = shadowUV * UVScale + UVOffset
        ------------------------------------------------------------
        */
        Core::Float2 UVScale;
        Core::Float2 UVOffset;
    };




    /*
    ============================================================
        Shadow Allocation Result
        One light may have multiple slices:
        - Spot        -> 1
        - Directional -> 1
        - Point       -> 6
        - CSM         -> 4
    ============================================================
    */
    struct ShadowAllocation
    {
        EShadowType ShadowType = EShadowType::Unknown;
        RenderCore::RenderTexture* Texture = nullptr;
        uint32_t TextureIndex = 0;
        std::vector<ShadowAllocationSlice> Slices;

        bool IsValid() const
        {
            return !Slices.empty();
        }
    };

    /*
    ============================================================
        Shadow Atlas Settings
    ============================================================
    */
    struct ShadowAtlasDesc
    {
        uint32_t Width = 4096;
        uint32_t Height = 4096;
    };

    /*
    ============================================================
        Shadow Allocator Config
    ============================================================
    */
    struct ShadowAllocatorDesc
    {
        ShadowAtlasDesc SpotShadowAtlas;
    };

    /*
    ============================================================
        Shadow Map Allocator
    ============================================================
    */
    class ShadowMapAllocator
    {
    public:
        ShadowMapAllocator();
        ~ShadowMapAllocator();

    public:
        bool Initialize(
            const ShadowAllocatorDesc& Desc);
        const ShadowAllocatorDesc& GetDesc() const;
        void Shutdown();

    public:
        /*
        ------------------------------------------------------------
            Dedicated allocations
        ------------------------------------------------------------
        */
        ShadowAllocation AllocateDirectionalCSMShadow(uint32_t Resolution, uint32_t CascadeCount);

        ShadowAllocation AllocatePointShadow(uint32_t Resolution);

        /*
        ------------------------------------------------------------
            Atlas allocations
        ------------------------------------------------------------
        */
        ShadowAllocation AllocateSpotShadow(uint32_t Resolution);

        RenderCore::RenderTexture* GetShadowAtlas() const;

        const std::vector<RenderCore::RenderTexture*>& GetDedicatedPointLightShadowTextures() const;
        const std::vector<RenderCore::RenderTexture*>& GetDedicatedParallelLightShadowTextures() const;

    public:
        void ReleaseShadow(
            ShadowAllocation& Allocation);

    private:
        bool InitializeAtlasResources();

        bool AllocateFromAtlas(
            uint32_t Width,
            uint32_t Height,
            ShadowAllocation& OutAllocation);

        bool AllocateDedicatedTexture(
            uint32_t Width,
            uint32_t Height,
			EShadowType ShadowType,
            ShadowAllocation& OutAllocation);

    private:
        ShadowAllocatorDesc Desc_;

    private:
        /*
        ------------------------------------------------------------
            Atlas resources
        ------------------------------------------------------------
        */
        RenderCore::RenderTextureSP ShadowAtlas_;

    private:
        /*
        ------------------------------------------------------------
            Dedicated shadow resources cache
        ------------------------------------------------------------
        */
        std::vector<RenderCore::RenderTextureSP> DedicatedShadowTextures_;
        std::vector<RenderCore::RenderTexture*> PointLightShadowTextures_;
        std::vector<RenderCore::RenderTexture*> ParallelLightShadowTextures_;
        uint32_t AtlasCursorX = 0;
        uint32_t AtlasCursorY = 0;
        uint32_t AtlasCurrentRowHeight = 0;
    };

}