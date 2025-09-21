#pragma once

#include "RenderGraphResource.h" // For RenderGraphResource
#include "RenderGraphPass.h"
#include <list>
#include <vector>
#include <unordered_map>

namespace RenderCore {


// -------------------------------------------------------------------------------------------------
//  Render Graph Builder (Now also managing passes and execution)
// -------------------------------------------------------------------------------------------------
class RenderGraphBuilder
{
public:
    // Construction/Destruction
    RenderGraphBuilder();
    ~RenderGraphBuilder();

    // Pass Management
    void AddPass(RenderGraphPass* pass);

    // Resource Creation (Examples - Add more as needed)
    RenderGraphTextureSP CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    RenderGraphBufferSP CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc);

    RenderGraphTextureSRVSP CreateTextureSRV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphBufferSRVSP CreateBufferSRV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphTextureUAVSP CreateTextureUAV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphBufferUAVSP CreateBufferUAV(const std::string& name, RenderGraphResourceSP resource);



    // Resource Access (Get RHI resources from RenderGraphResources)
    RHITextureSP GetTexture(RenderGraphResourceSP resource);
    RHIBufferSP GetBuffer(RenderGraphResourceSP resource);




    // Execution
    void Execute(); // Executes all added passes

private:
    using PassList = std::list<RenderGraphPass*>; // Using std::list for pass management
    using PassListGroup = std::list<PassList>; // Group of passes for execution


    PassList Passes; // List of passes to execute (using std::list)

    

    // Resource Cache
    std::unordered_map<std::string, RenderGraphTextureSP> TextureCache; // Cache for textures
    std::unordered_map<std::string, RenderGraphBufferSP> BufferCache;   // Cache for buffers


};

} // namespace WR::RenderCore