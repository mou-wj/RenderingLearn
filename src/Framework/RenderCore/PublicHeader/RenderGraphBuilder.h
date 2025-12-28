#pragma once

#include "RenderGraphResource.h" // For RenderGraphResource
#include "RenderGraphPass.h"
#include <list>
#include <vector>
#include <unordered_map>

namespace RenderCore {
struct RENDERCORE_API GraphResourceAccessState {
    RenderGraphPass* LastVisitor = nullptr;
    RenderGraphResourceSP Resource;
    struct TextureAccess {
        ERHIResourceAccess Access;
        RHITextureRegion TextureRegion;
    };
    struct BufferAccess {
        ERHIResourceAccess Access;
        RHIBufferRegion BufferRegion;
    };
    std::vector<TextureAccess> TextureAccesses;
    std::vector<BufferAccess> BufferAccesses;
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Builder (Now also managing passes and execution)
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API RenderGraphBuilder
{
public:
    // Construction/Destruction
    RenderGraphBuilder();
    ~RenderGraphBuilder();

    // Pass Management
    RenderGraphPassSP AddPass(const std::string& name, const RenderGraphPassInfo& info, std::function<void(RHICommandList&)>&& lambda);
    void AddPassDependency(RenderGraphPass* pass, RenderGraphPass* passConsumer);

    // Resource Creation (Examples - Add more as needed)
    RenderGraphTextureSP CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    RenderGraphBufferSP CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc);

    RenderGraphTextureSRVSP CreateTextureSRV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphBufferSRVSP CreateBufferSRV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphTextureUAVSP CreateTextureUAV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphBufferUAVSP CreateBufferUAV(const std::string& name, RenderGraphResourceSP resource);

    RenderGraphTextureSP RegisterExternalTexture(const std::string& name, RHITextureSP texture);

    // Resource Access (Get RHI resources from RenderGraphResources)
    RHITextureSP GetTexture(RenderGraphResourceSP resource);
    RHIBufferSP GetBuffer(RenderGraphResourceSP resource);

    template<typename T>
    T* AllocateParameter(){
        T* ptr = new T();
        ParameterCache.push_back(ptr);
        return ptr;
    }



    // Execution
    void Execute(); // Executes all added passes
protected:
    void AnalyzePasses(); // Analyzes passes and groups them for execution

private:
    using PassList = std::list<RenderGraphPass*>; // Using std::list for pass management
    using PassListGroup = std::list<PassList>; // Group of passes for execution
    std::list<RenderGraphPassSP> PasseSPs; // List of passes to execute (using std::list)
    PassList Passes; // List of passes to execute (using std::list)
    PassListGroup ParallelPasses; // Group of passes to execute in parallel (using std::list)
    // Resource Cache
    std::unordered_map<std::string, RenderGraphTextureSP> TextureCache; // Cache for textures
    std::unordered_map<std::string, RenderGraphBufferSP> BufferCache;   // Cache for buffers
    //Parameters Storage
    std::vector<void*> ParameterCache; // Cache for parameters
    //
    std::unordered_map<std::string, RenderGraphTextureSP> ExternalTextureCache; // Cache for resources
};

} // namespace WR::RenderCore