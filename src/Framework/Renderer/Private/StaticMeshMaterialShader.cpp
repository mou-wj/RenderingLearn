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
        return StaticMeshMaterialShaderPSParameters::GetMetaData();
    }

    IMPLEMENT_MESH_MATERIAL_SHADER_TYPE(
        StaticMeshMaterialShaderPS,
        "StaticMeshMaterialShaderPS",
        "/material/StaticMeshMaterialShaderPS.sf",
        "MainPS",
        ERHIShaderFrequency::Fragment
    )

        /*
           ===============================================================================
               StaticMeshMaterialShaderPS
           ===============================================================================
           */

        bool StaticMeshMaterialGBufferShaderPS::ShouldCompilePermutation(
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

    void StaticMeshMaterialGBufferShaderPS::ModifyShaderCompilerEnvironment(
        const RenderCore::ShaderPermutationParameters& Parameters,
        RenderCore::ShaderCompilerEnvironment& OutEnvironment)
    {
        const MeshMaterialShaderPermutationParameters& MeshParams =
            static_cast<const MeshMaterialShaderPermutationParameters&>(Parameters);

        OutEnvironment.SetDefine("PIXEL_SHADER", 1);
        Engine::ModifyShaderCompilerEnvironment(MeshParams.MaterialParams, OutEnvironment);
    }



    const RenderCore::ShaderParametersMetadata*
        StaticMeshMaterialGBufferShaderPS::GetShaderParameterMetadata()
    {
        return StaticMeshMaterialGBufferShaderPSParameters::GetMetaData();
    }

    IMPLEMENT_MESH_MATERIAL_SHADER_TYPE(
        StaticMeshMaterialGBufferShaderPS,
        "StaticMeshMaterialGBufferShaderPS",
        "/material/StaticMeshMaterialGBufferShaderPS.sf",
        "MainPS",
        ERHIShaderFrequency::Fragment
    )
    void StaticMeshMaterialDefferedShadingCS::ModifyShaderCompilerEnvironment(const RenderCore::ShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment)
    {
        const MaterialShaderPermutationParameters& MeshParams =
            static_cast<const MaterialShaderPermutationParameters&>(Parameters);
        Engine::ModifyShaderCompilerEnvironment(MeshParams.MaterialParams, OutEnvironment);
    }

    IMPLEMENT_MATERIAL_SHADER_TYPE(
        StaticMeshMaterialDefferedShadingCS,
        "StaticMeshMaterialDefferedShadingCS",
        "/material/StaticMeshMaterialDefferedShadingCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );
    
    
    /*
     ===============================================================================
         StaticMeshMaterialLightShadowPassPS
     ===============================================================================
     */
    
    bool StaticMeshMaterialLightShadowPassPS::ShouldCompilePermutation(
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
    
    void StaticMeshMaterialLightShadowPassPS::ModifyShaderCompilerEnvironment(
        const RenderCore::ShaderPermutationParameters& Parameters,
        RenderCore::ShaderCompilerEnvironment& OutEnvironment)
    {
        const MeshMaterialShaderPermutationParameters& MeshParams =
            static_cast<const MeshMaterialShaderPermutationParameters&>(Parameters);
    
        OutEnvironment.SetDefine("PIXEL_SHADER", 1);
        Engine::ModifyShaderCompilerEnvironment(MeshParams.MaterialParams, OutEnvironment);
        PermutationDomain Domain;
        Domain.SetFromId(Parameters.PermutationId);
        Domain.ModifyCompilationEnvironment(OutEnvironment);
    }
    
    
    
    const RenderCore::ShaderParametersMetadata*
        StaticMeshMaterialLightShadowPassPS::GetShaderParameterMetadata()
    {
        return StaticMeshMaterialGBufferShaderPSParameters::GetMetaData();
    }
    
    IMPLEMENT_MESH_MATERIAL_SHADER_TYPE(
        StaticMeshMaterialLightShadowPassPS,
        "StaticMeshMaterialLightShadowPassPS",
        "/material/StaticMeshMaterialLightShadowPassPS.sf",
        "MainPS",
        ERHIShaderFrequency::Fragment
    )
}