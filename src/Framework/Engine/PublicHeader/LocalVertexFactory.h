#pragma once

#include "VertexFactory.h"
#include "ShaderParameter.h"

namespace Engine
{



    BEGIN_SHADER_PARAMETER_STRUCT(LocalVertexFactoryParameters)
        SHADER_PARAMETER(Core::Float4x4, LocalToWorld)
        SHADER_PARAMETER(Core::Float4x4, WorldToLocal)
        SHADER_PARAMETER(Core::Float4x4, ViewProjection)
        SHADER_PARAMETER(Core::Float3, CameraWorldPosition)
    END_SHADER_PARAMETER_STRUCT(LocalVertexFactoryParameters)

    union LocalVertexFactoryFeatureFlags
    {
        uint64_t PackedFlags = 0;

        struct
        {
            uint64_t SupportsTexCoord : 1;
            uint64_t SupportsNormal : 1;
            uint64_t SupportsTangent : 1;
            uint64_t SupportsVertexColor : 1;
            uint64_t SupportsInstanceData : 1;
        };
    };


    //
    // Mesh Vertex ������Դ
    //
    struct LocalVertexFactoryData
    {
        RenderCore::VertexStreamComponent PositionComponent;
        RenderCore::VertexStreamComponent UVComponent;
        RenderCore::VertexStreamComponent NormalComponent;
        RenderCore::VertexStreamComponent TangentComponent;
        RenderCore::VertexStreamComponent ColorComponent;
    };

    //
    // LocalVertexFactory
    //
    class LocalVertexFactory : public RenderCore::VertexFactory
    {
        DECLARE_VERTEX_FACTORY_TYPE(LocalVertexFactory)
    public:
        LocalVertexFactory() = default;

        void SetData(const LocalVertexFactoryData& InData);

    public:

        static bool ShouldCompilePermutation(const RenderCore::VertexFactoryShaderPermutationParameters& Parameters);
        static void ModifyCompilationEnvironment(const RenderCore::VertexFactoryShaderPermutationParameters& Parameters, RenderCore::ShaderCompilerEnvironment& OutEnvironment);
        static const RenderCore::ShaderParametersMetadata* GetShaderParameterMetadata()
        {
            return LocalVertexFactoryParameters::GetMetaData();
        }
    private:


        LocalVertexFactoryData Data;
    };

}