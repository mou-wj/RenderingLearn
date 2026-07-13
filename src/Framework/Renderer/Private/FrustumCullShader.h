#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
#include "ShaderCore.h"

namespace Renderer {

    BEGIN_SHADER_PARAMETER_STRUCT(AABBParameters)
        SHADER_PARAMETER(Core::Float3 , Min)
        SHADER_PARAMETER(Core::Float3 , Max)
    END_SHADER_PARAMETER_STRUCT(AABBParameters)

    BEGIN_SHADER_PARAMETER_STRUCT(FrustumCullParameters)
        SHADER_PARAMETER(Core::Float4x4, ViewProjection)
        SHADER_PARAMETER(uint32_t, PrimitiveCount)
        SHADER_PARAMETER_RDG_STRUCTURED_BUFFER(AABBParameters, AABBs)
        SHADER_PARAMETER_RHI_UAV(RWStructuredBuffer<uint>, VisibilityFlags)
    END_SHADER_PARAMETER_STRUCT(FrustumCullParameters)

    class FrustumCullCS : public RenderCore::GlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER_TYPE(FrustumCullCS);

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
        {
            // no extra defines for now
        }

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return FrustumCullParameters::GetMetaData();
        }
    };

} // namespace Renderer
