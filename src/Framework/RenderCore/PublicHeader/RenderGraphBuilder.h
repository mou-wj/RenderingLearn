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
    void AddPassDependency(RenderGraphPass* dependentPass, RenderGraphPass* dependencyPass); // New: Adds a dependency between two passes

    // Resource Creation (Examples - Add more as needed)
    RenderGraphTextureSP CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    RenderGraphBufferSP CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc);

    // Resource Access (Get RHI resources from RenderGraphResources)
    RHITextureSP GetTexture(RenderGraphResource* resource);
    RHIBufferSP GetBuffer(RenderGraphResource* resource);

    // Execution
    void Execute(); // Executes all added passes

private:
    using PassList = std::list<RenderGraphPass*>; // Using std::list for pass management
    using PassListGroup = std::list<PassList>; // Group of passes for execution


    PassList Passes; // List of passes to execute (using std::list)

    // Dependency Graph: Stores dependencies between passes.
    // Key: A pass that depends on other passes.
    // Value: A list of passes that the key pass depends on.
    std::unordered_map<RenderGraphPass*, std::vector<RenderGraphPass*>> PassDependencies;
    

    // Resource Cache
    std::unordered_map<std::string, RenderGraphTextureSP> TextureCache; // Cache for textures
    std::unordered_map<std::string, RenderGraphBufferSP> BufferCache;   // Cache for buffers


};

} // namespace WR::RenderCore