#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include "RenderGraphResource.h"
#include "TaskPool.h"
#include <unordered_set>
#include <queue>

namespace RenderCore {

    RenderGraphBuilder::RenderGraphBuilder()
    {
    }

    RenderGraphBuilder::~RenderGraphBuilder()
    {
        // Clean up passes
        Passes.clear();
        PasseSPs.clear();
    }

    RenderGraphPassSP RenderGraphBuilder::AddPass(const std::string& name, const RenderGraphPassInfo& info, std::function<void(RHICommandList&)>&& lambda)
    {
        auto pass = std::make_shared<RenderGraphLambdaPass>(name, info, std::move(lambda));
        PasseSPs.push_back(pass);
        Passes.push_back(pass.get());
        return pass;
    }

    void RenderGraphBuilder::AddPassDependency(RenderGraphPass* pass, RenderGraphPass* passConsumer)
    {
        pass->PassConsumers.push_back(passConsumer);
        passConsumer->PassProducers.push_back(pass);
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
    RenderGraphTextureSP RenderGraphBuilder::RegisterExternalTexture(const std::string& name, RHITextureSP texture)
    {
        return RenderGraphTextureSP();
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
        using namespace RHI;
        // use Common::TaskPool from Common/TaskPool.h
        Core::TaskPool taskPool(std::thread::hardware_concurrency());

        std::vector<Core::TaskHandle> handles;
        handles.reserve(ParallelPasses.size());

        // For each parallel group, create one task that creates a command context/list
        // and executes each pass in the group sequentially: begin barriers, pass work, end barriers.
        for (const auto& group : ParallelPasses)
        {
            // capture a copy of the group
            std::vector<RenderGraphPass*> groupCopy;
            groupCopy.reserve(group.size());
            for (auto* p : group) groupCopy.push_back(p);

            auto handle = taskPool.AddTask([groupCopy = std::move(groupCopy)]() {
                // create a command context from global RHI
                RHI::RHIApi* api = RHI::GetGlobalRHIApi();
                if (!api) return;

                RHI::RHICommandContexSP ctx = api->CreateCommandContex();
                if (!ctx) return;

                RHI::RHICommandListSP cmdListSP = ctx->GetCommandList();
                if (!cmdListSP) return;

                RHI::RHICommandList& cmdList = *cmdListSP;

                for (auto* pass : groupCopy)
                {
                    if (!pass) continue;

                    // Execute begin barriers
                    pass->BeginBarrier.Execute(cmdList);

                    // Execute pass (uses RHICommandList)
                    try {
                        pass->Execute(cmdList);
                    } catch (...) {
                        // swallow; user can log/handle
                    }

                    // Execute end barriers
                    //pass->EndBarrier.Execute(cmdList);
                }

                // Optionally submit the command list via ctx / api if required by RHI.
            });

            handles.push_back(handle);
        }

        // wait for all groups to finish
        taskPool.WaitAll(handles);
    }

    void RenderGraphBuilder::AnalyzePasses()
    {
        // Map resource name -> last access state & last visitor
        struct LocalResState {
            RenderGraphPass* LastVisitor = nullptr;
            ERHIResourceAccess LastAccess = ERHIResourceAccess::Unknown;
            RenderGraphResourceSP Resource;
        };
        std::unordered_map<std::string, LocalResState> resStates;

        // Helper to register producer/consumer and add barrier on previous visitor
        auto handleAccess = [&](RenderGraphPass* pass, RenderGraphResourceSP resource, ERHIResourceAccess access) {
            if (!resource) return;
            const std::string& rname = resource->GetName();
            auto& state = resStates[rname];

            // If previously visited by another pass, insert a transition from previous access -> current
            if (state.LastVisitor && state.LastVisitor != pass) {
                // create transient info describing transition
                RHI::RHITransientInfo transient{};
                // determine type
                if (std::dynamic_pointer_cast<RenderGraphTexture>(resource)) {
                    transient.Type = RHI::RHITransientInfo::EType::Texture;
                    // fill texture range defaults already in struct ctor
                } else if (std::dynamic_pointer_cast<RenderGraphBuffer>(resource)) {
                    transient.Type = RHI::RHITransientInfo::EType::Buffer;
                } else {
                    transient.Type = RHI::RHITransientInfo::EType::Unknown;
                }

                transient.Resource = nullptr; // underlying RHI resource may be created later
                transient.AccessBefore = state.LastAccess;
                transient.AccessAfter = access;

                // add transition to previous visitor's end barrier
                state.LastVisitor->EndBarrier.AddTransition(transient);

                // link producer/consumer
                state.LastVisitor->PassConsumers.push_back(pass);
                pass->PassProducers.push_back(state.LastVisitor);
            }

            // update state
            state.LastVisitor = pass;
            state.LastAccess = access;
            state.Resource = resource;
        };

        // Iterate passes in order and examine their cached resource references
        for (auto* pass : Passes) {
            // collect resources accessed by this pass and their access type
            // Textures:
            if (pass->ReadTextureResourceCache) {
                handleAccess(pass, pass->ReadTextureResourceCache, ERHIResourceAccess::Read);
            }
            if (pass->ReadOnlyTextureCache) {
                // SRV -> read
                auto tex = pass->ReadOnlyTextureCache->GetDesc().Texture;
                if (tex) handleAccess(pass, tex, ERHIResourceAccess::Read);
            }
            if (pass->ReadWriteTextureCache) {
                // UAV -> read/write
                auto tex = pass->ReadWriteTextureCache->GetDesc().Texture;
                if (tex) handleAccess(pass, tex, ERHIResourceAccess::ReadWrite);
            }

            // Buffers:
            if (pass->ReadWriteBufferResourceCache) {
                handleAccess(pass, pass->ReadWriteBufferResourceCache, ERHIResourceAccess::ReadWrite);
            }
            if (pass->ReadOnlyBufferCache) {
                auto buf = pass->ReadOnlyBufferCache->GetDesc().Buffer;
                if (buf) handleAccess(pass, buf, ERHIResourceAccess::Read);
            }
            if (pass->ReadWriteBufferCache) {
                auto buf = pass->ReadWriteBufferCache->GetDesc().Buffer;
                if (buf) handleAccess(pass, buf, ERHIResourceAccess::ReadWrite);
            }

            // Render targets / other resources could be handled similarly (omitted for brevity)
        }

        // Build dependency graph (Kahn's algorithm) using PassProducers/PassConsumers
        std::unordered_map<RenderGraphPass*, size_t> indegree;
        std::unordered_set<RenderGraphPass*> allNodes;
        for (auto* pass : Passes) {
            allNodes.insert(pass);
            indegree[pass] = pass->PassProducers.size();
        }

        std::queue<RenderGraphPass*> zeroQueue;
        for (auto* node : allNodes) {
            if (indegree[node] == 0) zeroQueue.push(node);
        }

        ParallelPasses.clear();
        std::unordered_set<RenderGraphPass*> processed;

        while (!allNodes.empty()) {
            std::vector<RenderGraphPass*> currentSet;
            if (zeroQueue.empty()) {
                // Defensive: break cycle by taking one node
                auto it = allNodes.begin();
                if (it == allNodes.end()) break;
                currentSet.push_back(*it);
            } else {
                size_t qCount = zeroQueue.size();
                for (size_t i = 0; i < qCount; ++i) {
                    auto* n = zeroQueue.front(); zeroQueue.pop();
                    if (processed.find(n) != processed.end()) continue;
                    currentSet.push_back(n);
                }
            }

            // create a PassList for this parallel group
            PassList groupList;
            for (auto* node : currentSet) {
                groupList.push_back(node);
                processed.insert(node);
                allNodes.erase(node);
            }

            // reduce indegree of consumers
            for (auto* node : currentSet) {
                for (auto* consumer : node->PassConsumers) {
                    auto it = indegree.find(consumer);
                    if (it == indegree.end()) continue;
                    if (it->second > 0) {
                        it->second--;
                        if (it->second == 0) zeroQueue.push(consumer);
                    }
                }
            }

            if (!groupList.empty()) {
                ParallelPasses.push_back(groupList);
            }
        }

        // Append any unprocessed nodes (cycle fallback)
        for (auto* remaining : Passes) {
            if (processed.find(remaining) == processed.end()) {
                PassList single;
                single.push_back(remaining);
                ParallelPasses.push_back(single);
            }
        }
    }


} // namespace RenderCore
