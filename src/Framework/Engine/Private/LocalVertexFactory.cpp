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
        // ���� Vertex Layout
        //
        void LocalVertexFactory::SetData(
            const LocalVertexFactoryData& InData)
    {
        Data = InData;

        //
        // ��վ� Streams
        //
        Streams.clear();

        //
        // ���� Vertex Elements
        //
        std::vector<VertexElement> Elements;

        //
        // POSITION -> location 0
        //
        auto AssignComponent = [&Elements,this](const RenderCore::VertexStreamComponent& Component, uint32_t& Location) {
            if (Component.Buffer != nullptr) {
                Elements.push_back(
                    AccessStreamComponent(
                        Component,
                        Location));
                Location++;
            }

        };
        uint32_t OptionalLocation = 0;
        AssignComponent(Data.PositionComponent, OptionalLocation);
        LocalVertexFactoryFeatureFlags flags;
		
        //
        // UV -> location 1
        //
        AssignComponent(Data.UVComponent, OptionalLocation);
        flags.SupportsTexCoord = Data.UVComponent.Buffer != nullptr;


        //
        // NORMAL -> location 2
        //
        AssignComponent(Data.NormalComponent, OptionalLocation);
        flags.SupportsNormal = Data.NormalComponent.Buffer != nullptr;
        //
        // TANGENT -> location 3
        //
        AssignComponent(Data.TangentComponent, OptionalLocation);
        flags.SupportsTangent = Data.TangentComponent.Buffer != nullptr;
        //
        // COLOR -> location 4
        //
        AssignComponent(Data.ColorComponent, OptionalLocation);
        flags.SupportsVertexColor = Data.ColorComponent.Buffer != nullptr;
        //
        // ���� Vulkan VertexInputState
        //
        InitDeclaration(Elements,&RHIVertexDescState);
        VertexFactoryFlags = flags.PackedFlags;
        //
        // Instance ID -> location 5
        //
        RenderCore::VertexStreamComponent InstanceComponent;
        InstanceComponent.Buffer = nullptr;
        InstanceComponent.InputRate = RHI::ERHIInputRate::PerInstance;
        InstanceComponent.Format = RHI::ERHIFormat::R32_UInt;
        InstanceComponent.Stride = sizeof(uint32_t);
        instanceLocation = OptionalLocation;
        Elements.push_back(
            AccessStreamComponent(
                InstanceComponent,
                OptionalLocation));
        //
        // ���� Vulkan VertexInputState
        //
        InitDeclaration(Elements, &RHIInstancedVertexDescState);
        

    }
    RHI::RHIVertexDescState* LocalVertexFactory::GetRHIInstancedVertexDescState() const {
        return RHIInstancedVertexDescState;
    }
    void LocalVertexFactory::BindInstanceBuffer(RHI::RHIGraphicCommandList& cmdList, RHI::RHIBuffer* InstanceBuffer, uint32_t Offset) const
    {
        if (instanceLocation < Streams.size())
            cmdList.SetStreamSource(Streams[instanceLocation].Binding, InstanceBuffer, Offset);
    }
    //
    // Shader Permutation
    //
    bool LocalVertexFactory::ShouldCompilePermutation(
        const VertexFactoryShaderPermutationParameters& Parameters)
    {
        LocalVertexFactoryFeatureFlags Flags = {};
        Flags.PackedFlags = Parameters.VertexFactoryFlags;
        
        return Flags.PackedFlags == 3 || Flags.PackedFlags == 19;
        
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
            "VF_SUPPORTS_INSTANCE",
            Flags.SupportsInstanceData);

    }


}        