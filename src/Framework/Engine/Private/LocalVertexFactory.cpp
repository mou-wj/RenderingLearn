#include "LocalVertexFactory.h"

using namespace RenderCore;
namespace Engine
{

    IMPLEMENT_VERTEX_FACTORY_TYPE(
        LocalVertexFactory,
        "/vertexfactory/LocalVertexFactory.sf",
        1 << 6
    )


        //
        // 设置 Vertex Layout
        //
        void LocalVertexFactory::SetData(
            const LocalVertexFactoryData& InData)
    {
        Data = InData;

        //
        // 清空旧 Streams
        //
        Streams.clear();

        //
        // 构建 Vertex Elements
        //
        std::vector<VertexElement> Elements;

        //
        // POSITION -> location 0
        //
        auto AssignComponent = [&Elements,this](const RenderCore::VertexStreamComponent& Component, uint32_t Location) {
            if (Component.Buffer != nullptr) {
                Elements.push_back(
                    AccessStreamComponent(
                        Component,
                        Location));
            }

        };
        AssignComponent(Data.PositionComponent, 0);
        LocalVertexFactoryFeatureFlags flags;
        //
        // UV -> location 1
        //
        AssignComponent(Data.UVComponent, 1);
        flags.SupportsTexCoord = Data.UVComponent.Buffer != nullptr;


        //
        // NORMAL -> location 2
        //
        AssignComponent(Data.NormalComponent, 2);
        flags.SupportsNormal = Data.NormalComponent.Buffer != nullptr;
        //
        // TANGENT -> location 3
        //
        AssignComponent(Data.TangentComponent, 3);
        flags.SupportsTangent = Data.TangentComponent.Buffer != nullptr;
        //
        // COLOR -> location 4
        //
        AssignComponent(Data.ColorComponent, 4);
        flags.SupportsVertexColor = Data.ColorComponent.Buffer != nullptr;
        //
        // 创建 Vulkan VertexInputState
        //
        InitDeclaration(Elements);
        VertexFactoryFlags = flags.PackedFlags;
    }


    //
    // Shader Permutation
    //
    bool LocalVertexFactory::ShouldCompilePermutation(
        const VertexFactoryShaderPermutationParameters& Parameters)
    {
        LocalVertexFactoryFeatureFlags Flags = {};
        Flags.PackedFlags = Parameters.VertexFactoryFlags;
        /*
           ===========================================================================
               Tangent requires Normal
           ===========================================================================
           */

        if (Flags.SupportsTangent &&
            !Flags.SupportsNormal)
        {
            return false;
        }

        /*
        ===========================================================================
            Instance data currently unsupported
        ===========================================================================
        */

        if (Flags.SupportsInstanceData)
        {
            return false;
        }

        return true;
    }


    //
    // Shader Compile Environment
    //
    void LocalVertexFactory::ModifyCompilationEnvironment(
        const VertexFactoryShaderPermutationParameters& Parameters,
        ShaderCompilerEnvironment& OutEnvironment)
    {

        LocalVertexFactoryFeatureFlags Flags = {};
		Flags.PackedFlags = Parameters.VertexFactoryFlags;

        /*
        ===========================================================================
            Vertex Factory Type
        ===========================================================================
        */

        OutEnvironment.SetDefine("LOCAL_VERTEX_FACTORY", 1);

        /*
        ===========================================================================
            Geometry Attribute Features
        ===========================================================================
        */

        OutEnvironment.SetDefine(
            "VF_SUPPORTS_TEXCOORD",
            Flags.SupportsTexCoord);

        OutEnvironment.SetDefine(
            "VF_SUPPORTS_NORMAL",
            Flags.SupportsNormal);

        OutEnvironment.SetDefine(
            "VF_SUPPORTS_TANGENT",
            Flags.SupportsTangent);

        OutEnvironment.SetDefine(
            "VF_SUPPORTS_VERTEX_COLOR",
            Flags.SupportsVertexColor);

        /*
        ===========================================================================
            Instance Features
        ===========================================================================
        */

        OutEnvironment.SetDefine(
            "VF_SUPPORTS_INSTANCE_DATA",
            Flags.SupportsInstanceData);

    }

}