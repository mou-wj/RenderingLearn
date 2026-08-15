#pragma once
#include "ShaderParameter.h"
#include <string>
#include <cstdint>

#include "RHIDefine.h"
#include "ShaderParameter.h"

namespace Renderer
{

    //=============================================================================
    // GBuffer Slot (Semantic Layer)
    //=============================================================================

    enum EGBufferSlot
    {
        GBS_Invalid = 0,

        GBS_BaseColor,
        GBS_WorldNormal,
        GBS_Roughness,
        GBS_Metallic,
        GBS_Specular,
        GBS_AO,
        GBS_Velocity,
        GBS_Depth,
        GBS_ShadingModelId,

        GBS_Num
    };

    //=============================================================================
    // GBuffer Layout Type
    //=============================================================================

    enum EGBufferLayout
    {
        GBL_Default = 0,
        GBL_ForceVelocity,

        GBL_Num
    };

    //=============================================================================
    // Semantic Mapping
    //
    // Example:
    // BaseColor -> Target0.rgb
    // Metallic  -> Target0.a
    //=============================================================================

    struct FGBufferItem
    {
        FGBufferItem()
        {
            bIsValid = false;
            BufferSlot = GBS_Invalid;
            TargetIndex = -1;
            ChannelStart = 0;
            ChannelCount = 0;
        }

        FGBufferItem(
            EGBufferSlot InSlot,
            int InTargetIndex,
            uint8_t InChannelStart,
            uint8_t InChannelCount)
        {
            bIsValid = true;
            BufferSlot = InSlot;
            TargetIndex = InTargetIndex;
            ChannelStart = InChannelStart;
            ChannelCount = InChannelCount;
        }

        bool bIsValid;

        EGBufferSlot BufferSlot;

        int TargetIndex;

        // 0 = R
        // 1 = G
        // 2 = B
        // 3 = A
        uint8_t ChannelStart;
        uint8_t ChannelCount;
    };

    //=============================================================================
    // Physical Render Target Description
    //=============================================================================

    struct FGBufferTarget
    {
        FGBufferTarget()
        {
            TargetFormat = RHI::ERHIFormat::Unknown;
            bIsSrgb = false;
            bIsRenderTargetable = true;
            bIsShaderResource = true;
        }

        FGBufferTarget(
            RHI::ERHIFormat InType,
            const std::string& InName,
            bool bInIsSrgb = false)
        {
            TargetFormat = InType;
            TargetName = InName;
            bIsSrgb = bInIsSrgb;
            bIsRenderTargetable = true;
            bIsShaderResource = true;
        }

        RHI::ERHIFormat TargetFormat;

        std::string TargetName;

        bool bIsSrgb;
        bool bIsRenderTargetable;
        bool bIsShaderResource;
    };

    //=============================================================================
    // Runtime Binding
    //=============================================================================

    struct FGBufferBinding
    {
        int Index = -1;
        RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
    };

    struct FGBufferBindingInfo {
        FGBufferBinding GBufferA;
        FGBufferBinding GBufferB;
        FGBufferBinding GBufferC;
    };

    //=============================================================================
    // Full GBuffer Layout Info
    //=============================================================================

    struct FGBufferInfo
    {
        static constexpr int MaxTargets = 8;

        FGBufferInfo()
        {
            Layout = GBL_Default;
            NumTargets = 0;
        }

        EGBufferLayout Layout;

        int NumTargets;

        FGBufferTarget Targets[MaxTargets];

        FGBufferItem Slots[GBS_Num];
    };

    //=============================================================================
    // Layout Creation Parameters
    //=============================================================================

    struct FGBufferParams
    {
        bool bHasVelocity = false;
        bool bHasAO = true;
        bool bHasSpecular = true;

        bool operator==(const FGBufferParams& RHS) const
        {
            return
                bHasVelocity == RHS.bHasVelocity &&
                bHasAO == RHS.bHasAO &&
                bHasSpecular == RHS.bHasSpecular;
        }

        bool operator!=(const FGBufferParams& RHS) const
        {
            return !(*this == RHS);
        }
    };

    //=============================================================================
    // Utility Functions
    //=============================================================================

    FGBufferInfo CreateGBufferInfo(const FGBufferParams& Params);

    const FGBufferItem* FindGBufferSlot(
        const FGBufferInfo& GBufferInfo,
        EGBufferSlot Slot);

    const FGBufferTarget* FindGBufferTargetByName(
        const FGBufferInfo& GBufferInfo,
        const std::string& Name);

    BEGIN_SHADER_PARAMETER_STRUCT(GBufferInputParameters)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferA)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferB)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferC)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Depth)
        SHADER_PARAMETER_SAMPLER(PointSampler)
    END_SHADER_PARAMETER_STRUCT(GBufferInputParameters)

}