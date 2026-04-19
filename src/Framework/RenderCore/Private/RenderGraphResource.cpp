#include "RenderGraphResource.h"
#include "RenderGraphBuilder.h"
#include "RHIApi.h"

namespace RenderCore {

// -------------------------------------------------------------------------------------------------
//  Render Graph Resource Base Class Implementation
// -------------------------------------------------------------------------------------------------
RenderGraphResource::RenderGraphResource(const std::string& name)
    : Name(name), bImported(false), bCreated(false), Resource(nullptr)
{
}

RenderGraphResource::~RenderGraphResource()
{
    // Cleanup if necessary
}

void RenderGraphResource::SetRHIResource(RHIResource* resource)
{
    Resource = resource;
}

// -------------------------------------------------------------------------------------------------
//  Render Graph Texture Implementation
// -------------------------------------------------------------------------------------------------
RenderGraphTexture::RenderGraphTexture(const std::string& name, const RenderGraphTextureDesc& desc)
    : RenderGraphResource(name), desc(desc)
{
}

RenderGraphTexture::~RenderGraphTexture()
{
    // Cleanup if necessary
}

void RenderGraphTexture::Create(RenderGraphBuilder& builder)
{
    if (!IsCreated())
    {
        RHITextureDesc textureDesc;
        RHITextureSP texture = GRHIApi->CreateTexture(textureDesc);
        SetRHITexture(texture.get());
        SetCreated(true);
    }
}

// -------------------------------------------------------------------------------------------------
//  Render Graph Buffer Implementation
// -------------------------------------------------------------------------------------------------
RenderGraphBuffer::RenderGraphBuffer(const std::string& name, const RenderGraphBufferDesc& desc)
    : RenderGraphResource(name), desc(desc)
{
}

RenderGraphBuffer::~RenderGraphBuffer()
{
    // Cleanup if necessary
}

void RenderGraphBuffer::Create(RenderGraphBuilder& builder)
{
    if (!IsCreated())
    {
        RHIBufferDesc bufferDesc;
        RHIBufferSP buffer = GRHIApi->CreateBuffer(bufferDesc);
        SetRHIBuffer(buffer.get());
        SetCreated(true);
    }
}

RenderGraphUAV::RenderGraphUAV(const std::string& name) : RenderGraphView(name)
{
}
RenderGraphUAV::~RenderGraphUAV()
{
}

RenderGraphSRV::RenderGraphSRV(const std::string& name) : RenderGraphView(name)
{
}
RenderGraphSRV::~RenderGraphSRV()
{
}

RenderGraphView::RenderGraphView(const std::string& name)
{
}
RenderGraphView::~RenderGraphView()
{
}
RenderGraphResource* RenderGraphView::GetResource() const
{
	return Resource;
}


RenderGraphTextureSRV::RenderGraphTextureSRV(const std::string& name, const RenderGraphTextureSRVDesc& desc)   
    : RenderGraphSRV(name), Desc(desc)
{   
}

RenderGraphTextureSRV::~RenderGraphTextureSRV()
{
    // Cleanup if necessary
}

RenderGraphTextureUAV::RenderGraphTextureUAV(const std::string& name, const RenderGraphTextureUAVDesc& desc)
    : RenderGraphUAV(name), Desc(desc)
{   
    // TODO: Implement
}

RenderGraphTextureUAV::~RenderGraphTextureUAV()
{
    // Cleanup if necessary

}

RenderGraphBufferSRV::RenderGraphBufferSRV(const std::string& name, const RenderGraphBufferSRVDesc& desc)
    : RenderGraphSRV(name), Desc(desc)
{   
    // TODO: Implement
}

RenderGraphBufferSRV::~RenderGraphBufferSRV()
{
    // Cleanup if necessary
}

RenderGraphBufferUAV::RenderGraphBufferUAV(const std::string& name, const RenderGraphBufferUAVDesc& desc)
    : RenderGraphUAV(name), Desc(desc)
{
    // TODO: Implement
}
RenderGraphBufferUAV::~RenderGraphBufferUAV()
{
    // Cleanup if necessary
}









RenderGraphTextureDesc RenderGraphTextureDesc::ConvertFrom(const RHI::RHITextureDesc& other)
{
    RenderGraphTextureDesc desc;
    (RHI::RHITextureDesc)desc = other;
    return desc;
}

} // namespace WR::RenderCore