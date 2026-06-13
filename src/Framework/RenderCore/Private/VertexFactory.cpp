#include "VertexFactory.h"
#include "RHIApi.h"
#include "RHIPipelineStateCache.h"
using namespace RHI;

namespace RenderCore {
	RHI::RHIVertexDescState* VertexFactory::GetRHIVertexDescState() const
	{
		return RHIVertexDescState;
	}

    void VertexFactory::Bind(RHI::RHIGraphicCommandList& RHICmdList) const
    {
        for (int i = 0; i < Streams.size(); i++) {
            RHICmdList.SetStreamSource(Streams[i].Binding, Streams[i].Buffer, Streams[i].Offset);
        }
    }

	VertexFactory::VertexElement VertexFactory::AccessStreamComponent(const VertexStreamComponent& Component, uint32_t AttributeIndex)
	{
        for (const auto& Stream : Streams)
        {
            bool bMatch =
                Stream.Buffer == Component.Buffer &&
                Stream.Stride == Component.Stride &&
                Stream.InputRate == Component.InputRate &&
                Stream.Offset == 0;

            if (bMatch)
            {
                return VertexElement
                {
                    .Binding = Stream.Binding,
                    .Location = AttributeIndex,
                    .ElementOffset = Component.ComponentOffset,
                    .Format = Component.Format
                };
            }
        }

        uint32_t NewBinding = (uint32_t)Streams.size();

        VertexStream NewStream;
        NewStream.Buffer = Component.Buffer;
        NewStream.Stride = Component.Stride;
        NewStream.Offset = 0;
        NewStream.Binding = NewBinding;
        NewStream.InputRate = Component.InputRate;

        Streams.push_back(NewStream);

        return VertexElement
        {
            .Binding = NewBinding,
            .Location = AttributeIndex,
            .ElementOffset = Component.ComponentOffset,
            .Format = Component.Format
        };
	}

	void VertexFactory::InitDeclaration(const std::vector<VertexElement>& Elements)
	{
        RHI::RHIVertexDescStateDesc Desc;

        //
        // 创建 Binding Descriptions
        //
        for (uint32_t i = 0; i < Streams.size(); ++i)
        {
            const VertexStream& Stream = Streams[i];

            RHI::RHIVertexBindingDesc BindingDesc;
            BindingDesc.binding = Stream.Binding;
            BindingDesc.stride = Stream.Stride;
            BindingDesc.inputRate = Stream.InputRate;

            Desc.bindings.push_back(BindingDesc);
        }

        //
        // 创建 Attribute Descriptions
        //
        for (const VertexElement& Element : Elements)
        {
            RHI::RHIVertexAttributeDesc AttrDesc;

            AttrDesc.location = Element.Location;
            AttrDesc.binding = Element.Binding;
            AttrDesc.offset = Element.ElementOffset;
            AttrDesc.format = Element.Format;

            Desc.attributes.push_back(AttrDesc);
        }

        //
        // 创建最终 VertexInputState
        //
        RHIVertexDescState = RHIPipelineStateCache::GetOrCreateVertexDescState(Desc);

	}



} // namespace RenderCore