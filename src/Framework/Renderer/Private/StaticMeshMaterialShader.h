// StaticMeshMaterialShader.h
#pragma once

#include "MeshMaterialShader.h"
#include "ShaderParameter.h"
#include "LocalVertexFactory.h"
#include "SceneShaderParameters.h"
#include "MaterialShaderParameter.h"
#include "MateiralShader.h"
#include "ShaderParameter.h"

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
        SHADER_PARAMETER_STRUCT_REFERENCE(SceneShaderParameters, Scene)
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


    BEGIN_SHADER_PARAMETER_STRUCT(StaticMeshMaterialGBufferShaderPSParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::LocalVertexFactoryParameters, vertexFactoryParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::MaterialShaderParameters, Material)
        SHADER_PARAMETER_RENDER_TARGET_BINDING_SLOTS(renderTargetSlots)
    END_SHADER_PARAMETER_STRUCT(StaticMeshMaterialGBufferShaderPSParameters)
    /*
    ===============================================================================
        StaticMeshMaterialGBufferShaderPS
    ===============================================================================
    */
    class StaticMeshMaterialGBufferShaderPS : public MeshMaterialShader
    {
    public:
        DECLARE_MESH_MATERIAL_SHADER_TYPE(StaticMeshMaterialGBufferShaderPS)


        static bool ShouldCompilePermutation(
            const RenderCore::ShaderPermutationParameters& Parameters);

        static void ModifyShaderCompilerEnvironment(
            const RenderCore::ShaderPermutationParameters& Parameters,
            RenderCore::ShaderCompilerEnvironment& OutEnvironment);

        static const RenderCore::ShaderParametersMetadata*
            GetShaderParameterMetadata();
    };

    BEGIN_SHADER_PARAMETER_STRUCT(GBufferTextureParameters)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferA)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferB)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferC)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Depth)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputColor)
        SHADER_PARAMETER_SAMPLER(PointSampler)
    END_SHADER_PARAMETER_STRUCT(GBufferTextureParameters)


    BEGIN_SHADER_PARAMETER_STRUCT(StaticMeshMaterialDefferedShadingCSParameters)
        SHADER_PARAMETER(Core::Float4x4, InvViewProj)
        SHADER_PARAMETER(Core::Float3, CameraPos)
        SHADER_PARAMETER(Core::Float2, ScreenSize)
        SHADER_PARAMETER_STRUCT_REFERENCE(SceneShaderParameters, Scene)
        SHADER_PARAMETER_STRUCT_INCLUDE(GBufferTextureParameters, GBuffer)
    END_SHADER_PARAMETER_STRUCT(StaticMeshMaterialDefferedShadingCSParameters)

    class StaticMeshMaterialDefferedShadingCS : public MaterialShader
    {

    public:

        DECLARE_MATERIAL_SHADER_TYPE(StaticMeshMaterialDefferedShadingCS);

        static bool ShouldCompilePermutation(const RenderCore::ShaderPermutationParameters& Parameters)
        {
            return true;
        }
        static void ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment);

        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return StaticMeshMaterialDefferedShadingCSParameters::GetMetaData();
        }
    };

    BEGIN_SHADER_PARAMETER_STRUCT(StaticMeshMaterialLightShadowPassPSParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(Engine::LocalVertexFactoryParameters, vertexFactoryParameters)
        SHADER_PARAMETER_RENDER_TARGET_BINDING_SLOTS(renderTargetSlots)
    END_SHADER_PARAMETER_STRUCT(StaticMeshMaterialLightShadowPassPSParameters)
    /*
    ===============================================================================
        StaticMeshMaterialLightShadowPassPS
    ===============================================================================
    */
    class StaticMeshMaterialLightShadowPassPS : public MeshMaterialShader
    {
    public:
        DECLARE_MESH_MATERIAL_SHADER_TYPE(StaticMeshMaterialLightShadowPassPS)
        static bool ShouldCompilePermutation(
            const RenderCore::ShaderPermutationParameters& Parameters);

        static void ModifyShaderCompilerEnvironment(
            const RenderCore::ShaderPermutationParameters& Parameters,
            RenderCore::ShaderCompilerEnvironment& OutEnvironment);

        static const RenderCore::ShaderParametersMetadata*
            GetShaderParameterMetadata();
    };

}