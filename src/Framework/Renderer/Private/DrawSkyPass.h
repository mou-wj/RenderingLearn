#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
#include "ShaderCore.h"

namespace Renderer {
    BEGIN_SHADER_PARAMETER_STRUCT(DrawSkyVertexParameters)
    END_SHADER_PARAMETER_STRUCT(DrawSkyVertexParameters)

    class DrawSkyVS : public RenderCore::GlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER_TYPE(DrawSkyVS)

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
        {
            OutEnvironment.SetDefine("VERTEX_SHADER", 1);
        }

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return DrawSkyVertexParameters::GetMetaData();
        }
    };

    BEGIN_SHADER_PARAMETER_STRUCT(DrawSkyPixelParameters)
        SHADER_PARAMETER_RHI_TEXTURE(Texture2D, EnvironmentMap)
        SHADER_PARAMETER_SAMPLER(EnvironmentMapSampler)
    END_SHADER_PARAMETER_STRUCT(DrawSkyPixelParameters)

    class DrawSkyPS : public RenderCore::GlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER_TYPE(DrawSkyPS)

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }

        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
        {
            OutEnvironment.SetDefine("PIXEL_SHADER", 1);
        }

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return DrawSkyPixelParameters::GetMetaData();
        }
    };
}
