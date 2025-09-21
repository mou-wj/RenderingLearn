#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include "RenderGraphResource.h"
#include <unordered_set>

namespace RenderCore {

    RenderGraphBuilder::RenderGraphBuilder()
    {
    }

    RenderGraphBuilder::~RenderGraphBuilder()
    {
        // Clean up passes
        for (auto& pass : Passes)
        {
            delete pass;
        }
        Passes.clear();
    }

    void RenderGraphBuilder::AddPass(RenderGraphPass* pass)
    {
        Passes.push_back(pass);
    }


    RenderGraphTextureSP RenderGraphBuilder::CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc)
    {
        // Check if the texture already exists in the cache
        auto it = TextureCache.find(name);
        if (it != TextureCache.end())
        {
            return it->second; // Return the cached texture
        }

        // Create a new texture and add it to the cache
        auto texture = std::make_shared<RenderGraphTexture>(name, desc);
        texture->Create(*this);
        TextureCache[name] = texture;
        return texture;
    }

    RenderGraphBufferSP RenderGraphBuilder::CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc)
    {
        // Check if the buffer already exists in the cache
        auto it = BufferCache.find(name);
        if (it != BufferCache.end())
        {
            return it->second; // Return the cached buffer
        }

        // Create a new buffer and add it to the cache
        auto buffer = std::make_shared<RenderGraphBuffer>(name, desc);
        buffer->Create(*this);
        BufferCache[name] = buffer;
        return buffer;
    }
    
    RenderGraphTextureSRVSP RenderGraphBuilder::CreateTextureSRV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto textureResource = std::dynamic_pointer_cast<RenderGraphTexture>(resource);
        if (!textureResource)
        {
            
        }

        RenderGraphTextureSRVDesc desc;
        desc.Texture = textureResource;

        return std::make_shared<RenderGraphTextureSRV>(name, desc);
    }

    RenderGraphBufferSRVSP RenderGraphBuilder::CreateBufferSRV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto bufferResource = std::dynamic_pointer_cast<RenderGraphBuffer>(resource);
        if (!bufferResource)
        {

        }

        RenderGraphBufferSRVDesc desc;
        desc.Buffer = bufferResource;


        return std::make_shared<RenderGraphBufferSRV>(name,desc);
    }

    RenderGraphTextureUAVSP RenderGraphBuilder::CreateTextureUAV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto textureResource = std::dynamic_pointer_cast<RenderGraphTexture>(resource);
        if (!textureResource)
        {

        }

        RenderGraphTextureUAVDesc desc;
        desc.Texture = textureResource;

        return std::make_shared<RenderGraphTextureUAV>(name,desc);
    }

    RenderGraphBufferUAVSP RenderGraphBuilder::CreateBufferUAV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto bufferResource = std::dynamic_pointer_cast<RenderGraphBuffer>(resource);
        if (!bufferResource)
        {
            
        }

        RenderGraphBufferUAVDesc desc;
        desc.Buffer = bufferResource;

        return std::make_shared<RenderGraphBufferUAV>(name, desc);
    }
    RHITextureSP RenderGraphBuilder::GetTexture(RenderGraphResourceSP resource)
    {
        auto textureResource = std::dynamic_pointer_cast<RenderGraphTexture>(resource);
        if (!textureResource)
        {
        }
        return textureResource->GetRHITexture();
    }

    RHIBufferSP RenderGraphBuilder::GetBuffer(RenderGraphResourceSP resource)
    {
        auto bufferResource = std::dynamic_pointer_cast<RenderGraphBuffer>(resource);
        if (!bufferResource)
        {

        }
        return bufferResource->GetRHIBuffer();
    }

    void RenderGraphBuilder::Execute()
    {
    }


}
