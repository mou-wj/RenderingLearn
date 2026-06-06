// StaticMeshMaterialShader.h
#pragma once

#include "MeshMaterialShader.h"
#include "ShaderParameter.h"
#include "LocalVertexFactory.h"
#include "SceneShaderParameters.h"
#include "MaterialShaderParameter.h"
#include "LocalVertexFactory.h"

namespace Renderer
{
    BEGIN_SHADER_PARAMETER_STRUCT(StaticMeshMaterialShaderVSParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::LocalVertexFactoryParameters, vertexFactoryParameters)
    END_SHADER_PARAMETER_STRUCT(StaticMeshMaterialShaderVSParameters)

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

    BEGIN_SHADER_PARAMETER_STRUCT(StaticMeshMaterialShaderPSParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::LocalVertexFactoryParameters, vertexFactoryParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::SceneShaderParameters, Scene)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::MaterialShaderParameters, Material)
        SHADER_PARAMETER_RENDER_TARGET_BINDING_SLOTS(renderTargetSlots)
    END_SHADER_PARAMETER_STRUCT(StaticMeshMaterialShaderPSParameters)
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