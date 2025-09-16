#include "RenderGraphResource.h"
#include "RenderGraphBuilder.h"
#include "RHIApi.h"

namespace RenderCore {

// -------------------------------------------------------------------------------------------------
//  Render Graph Resource Base Class Implementation
// -------------------------------------------------------------------------------------------------
RenderGraphResource::RenderGraphResource(const std::string& name)
    : Name(name), bImported(false), bCreated(false), RHIResource(nullptr)
{
}

RenderGraphResource::~RenderGraphResource()
{
    // Cleanup if necessary
}

void RenderGraphResource::SetRHIResource(RHIResourceSP resource)
{
    RHIResource = resource;
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
        RHITextureSP texture = GetGlobalRHIApi()->CreateTexture(textureDesc);
        SetRHITexture(texture);
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
        RHIBufferSP buffer = GetGlobalRHIApi()->CreateBuffer(bufferDesc);
        SetRHIBuffer(buffer);
        SetCreated(true);
    }
}

} // namespace WR::RenderCore