// StaticMeshMaterialShader.h
#pragma once

#include "MeshMaterialShader.h"

namespace Renderer
{
    /*
    ===============================================================================
        StaticMeshMaterialShaderVS
    ===============================================================================
    */
    class StaticMeshMaterialShaderVS : public MeshMaterialShader
    {
    public:
        DECLARE_MESH_MATERIAL_SHADER_TYPE(StaticMeshMaterialShaderVS)


        static bool ShouldCompilePermutation(
            const RenderCore::ShaderPermutationParameters& Parameters);

        static void ModifyShaderCompilerEnvironment(
            const RenderCore::ShaderPermutationParameters& Parameters,
            RenderCore::ShaderCompilerEnvironment& OutEnvironment);

        static const RenderCore::ShaderParametersMetadata*
            GetShaderParameterMetadata();
    };


    /*
    ===============================================================================
        StaticMeshMaterialShaderPS
    ===============================================================================
    */
    class StaticMeshMaterialShaderPS : public MeshMaterialShader
    {
    public:
        DECLARE_MESH_MATERIAL_SHADER_TYPE(StaticMeshMaterialShaderPS)


        static bool ShouldCompilePermutation(
            const RenderCore::ShaderPermutationParameters& Parameters);

        static void ModifyShaderCompilerEnvironment(
            const RenderCore::ShaderPermutationParameters& Parameters,
            RenderCore::ShaderCompilerEnvironment& OutEnvironment);

        static const RenderCore::ShaderParametersMetadata*
            GetShaderParameterMetadata();
    };

}