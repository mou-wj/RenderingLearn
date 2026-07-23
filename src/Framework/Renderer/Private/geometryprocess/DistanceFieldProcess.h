#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
#include "ShaderCore.h"
#include "RenderResource.h"

namespace Renderer {

    BEGIN_SHADER_PARAMETER_STRUCT(SDFVertexPosition)
        SHADER_PARAMETER(Core::Float3, Position)
    END_SHADER_PARAMETER_STRUCT(SDFVertexPosition)

    BEGIN_SHADER_PARAMETER_STRUCT(DistanceFieldVoxelizeParameters)
        SHADER_PARAMETER(Core::Float4x4, WorldToVoxel)
        SHADER_PARAMETER(Core::Int3, GridResolution)
        SHADER_PARAMETER(uint32_t, PrimitiveCount)
        SHADER_PARAMETER_RHI_STRUCTURED_BUFFER(SDFVertexPosition, VertexBuffer)
        SHADER_PARAMETER_RHI_SRV(StructuredBuffer<uint>, IndexBuffer)
        SHADER_PARAMETER_RHI_UAV(RWTexture3D<float>, OutputSDFTexture)
    END_SHADER_PARAMETER_STRUCT(DistanceFieldVoxelizeParameters)

    class DistanceFieldVoxelizeCS : public RenderCore::GlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER_TYPE(DistanceFieldVoxelizeCS);

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
        {
        }

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return DistanceFieldVoxelizeParameters::GetMetaData();
        }
    };

    BEGIN_SHADER_PARAMETER_STRUCT(DistanceFieldMergeParameters)
        SHADER_PARAMETER(Core::Float4x4, SourceToOutput)
        SHADER_PARAMETER(Core::Float4x4, OutputToSource)
        SHADER_PARAMETER(Core::Int3, OutputResolution)
        SHADER_PARAMETER_RHI_SRV(Texture3D<float>, InputSDFTexture)
        SHADER_PARAMETER_RHI_UAV(RWTexture3D<float>, OutputSDFTexture)
        SHADER_PARAMETER_SAMPLER(InputSDFSampler)
    END_SHADER_PARAMETER_STRUCT(DistanceFieldMergeParameters)

    class DistanceFieldMergeCS : public RenderCore::GlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER_TYPE(DistanceFieldMergeCS);

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
        {
        }

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return DistanceFieldMergeParameters::GetMetaData();
        }
    };

    BEGIN_SHADER_PARAMETER_STRUCT(DistanceFieldJumpFlood3DParameters)
        SHADER_PARAMETER(Core::Int3, GridResolution)
        SHADER_PARAMETER(uint32_t, JumpStep)
        SHADER_PARAMETER(uint32_t, bInitializeFromMask)
        SHADER_PARAMETER(uint32_t, bFinalizeDistance)
        SHADER_PARAMETER_RHI_SRV(Texture3D<float>, SurfaceMaskTexture)
        SHADER_PARAMETER_RHI_SRV(Texture3D<float4>, InputSeedTexture)
        SHADER_PARAMETER_RHI_UAV(RWTexture3D<float4>, OutputSeedTexture)
        SHADER_PARAMETER_RHI_UAV(RWTexture3D<float>, OutputDistanceTexture)
    END_SHADER_PARAMETER_STRUCT(DistanceFieldJumpFlood3DParameters)

    class DistanceFieldJumpFlood3DCS : public RenderCore::GlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER_TYPE(DistanceFieldJumpFlood3DCS);

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
        {
        }

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return DistanceFieldJumpFlood3DParameters::GetMetaData();
        }
    };

    struct DistanceFieldVoxelizePassInput
    {
        RenderCore::RenderBuffer* VertexBuffer = nullptr;
        uint32_t VertexCount = 0;
        RenderCore::RenderBuffer* IndexBuffer = nullptr;
        uint32_t IndexCount = 0;
        uint32_t PrimitiveCount = 0;
        Core::Float4x4 WorldToVoxel;
        Core::Int3 GridResolution = Core::Int3(0, 0, 0);
        RenderCore::RenderTexture* OutputSDFTexture = nullptr;
    };

    struct DistanceFieldMergePassInput
    {
        RenderCore::RenderTexture* InputSDFTexture = nullptr;
        RenderCore::RenderTexture* OutputSDFTexture = nullptr;
        Core::Float4x4 SourceToOutput;
        Core::Float4x4 OutputToSource;
        Core::Int3 OutputResolution = Core::Int3(0, 0, 0);
    };

    struct DistanceFieldJumpFlood3DPassInput
    {
        RenderCore::RenderTexture* SurfaceMaskTexture = nullptr;
        RenderCore::RenderTexture* OutputDistanceTexture = nullptr;
        Core::Int3 GridResolution = Core::Int3(0, 0, 0);
    };

    RENDERER_API bool ExecuteDistanceFieldVoxelizePass(const DistanceFieldVoxelizePassInput& Input);
    RENDERER_API bool ExecuteDistanceFieldMergePass(const DistanceFieldMergePassInput& Input);
    RENDERER_API bool ExecuteDistanceFieldJumpFlood3DPass(const DistanceFieldJumpFlood3DPassInput& Input);

} // namespace Renderer
