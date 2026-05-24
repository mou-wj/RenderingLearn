#include "StaticMeshMaterialShader.h"
#include "VertexFactory.h"
#include "ShaderCore.h"
#include "MaterialCore.h"
#include "LocalVertexFactory.h"
using namespace Engine;
namespace Renderer
{

    /*
    ===============================================================================
        StaticMeshMaterialShaderVS
    ===============================================================================
    */

    bool StaticMeshMaterialShaderVS::ShouldCompilePermutation(
        const RenderCore::ShaderPermutationParameters& Parameters)
    {
        const MeshMaterialShaderPermutationParameters& MeshParams =
            static_cast<const MeshMaterialShaderPermutationParameters&>(Parameters);

        // 这里只做最基础的过滤
        if (MeshParams.VFType == nullptr)
        {
            return false;
        }

        return true;
    }

    void StaticMeshMaterialShaderVS::ModifyShaderCompilerEnvironment(
        const RenderCore::ShaderPermutationParameters& Parameters,
        RenderCore::ShaderCompilerEnvironment& OutEnvironment)
    {
        const MeshMaterialShaderPermutationParameters& MeshParams =
            static_cast<const MeshMaterialShaderPermutationParameters&>(Parameters);

        // VS define
        OutEnvironment.SetDefine("VERTEX_SHADER", 1);



    }

    const RenderCore::ShaderParametersMetadata*
        StaticMeshMaterialShaderVS::GetShaderParameterMetadata()
    {
        return LocalVertexFactoryParameters::GetMetaData();
    }

    IMPLEMENT_MESH_MATERIAL_SHADER_TYPE(
        StaticMeshMaterialShaderVS,
        "StaticMeshMaterialShaderVS",
        "/material/StaticMeshMaterialShaderVS.sf",
        "MainVS",
        RHI::ERHIShaderFrequency::Vertex
    );




        /*
        ===============================================================================
            StaticMeshMaterialShaderPS
        ===============================================================================
        */

        bool StaticMeshMaterialShaderPS::ShouldCompilePermutation(
            const RenderCore::ShaderPermutationParameters& Parameters)
    {
        const MeshMaterialShaderPermutationParameters& MeshParams =
            static_cast<const MeshMaterialShaderPermutationParameters&>(Parameters);

        if (MeshParams.VFType == nullptr)
        {
            return false;
        }

        return true;
    }

    void StaticMeshMaterialShaderPS::ModifyShaderCompilerEnvironment(
        const RenderCore::ShaderPermutationParameters& Parameters,
        RenderCore::ShaderCompilerEnvironment& OutEnvironment)
    {
        const MeshMaterialShaderPermutationParameters& MeshParams =
            static_cast<const MeshMaterialShaderPermutationParameters&>(Parameters);

        OutEnvironment.SetDefine("PIXEL_SHADER", 1);
        Engine::ModifyShaderCompilerEnvironment(MeshParams.MaterialParams, OutEnvironment);
    }

    const RenderCore::ShaderParametersMetadata*
        StaticMeshMaterialShaderPS::GetShaderParameterMetadata()
    {
        return LocalVertexFactoryParameters::GetMetaData();
    }

    IMPLEMENT_MESH_MATERIAL_SHADER_TYPE(
        StaticMeshMaterialShaderPS,
        "StaticMeshMaterialShaderPS",
        "/material/StaticMeshMaterialShaderPS.sf",
        "MainPS",
        ERHIShaderFrequency::Fragment
    )

}