#include "RenderGraphBuilder.h"
#include "RHIApi.h"

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

    void RenderGraphBuilder::AddPassDependency(RenderGraphPass* dependentPass, RenderGraphPass* dependencyPass)
    {
        PassDependencies[dependentPass].push_back(dependencyPass);
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

    void RenderGraphBuilder::Execute()
    {
    }


}
