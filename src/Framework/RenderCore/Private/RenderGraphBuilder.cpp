#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include "RenderGraphResource.h"
#include "TaskPool.h"
#include "RenderThread.h"
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
        desc.Texture = textureResource.get();

        return std::make_shared<RenderGraphTextureSRV>(name, desc);
    }

    RenderGraphBufferSRVSP RenderGraphBuilder::CreateBufferSRV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto bufferResource = std::dynamic_pointer_cast<RenderGraphBuffer>(resource);
        if (!bufferResource)
        {

        }

        RenderGraphBufferSRVDesc desc;
        desc.Buffer = bufferResource.get();


        return std::make_shared<RenderGraphBufferSRV>(name,desc);
    }

    RenderGraphTextureUAVSP RenderGraphBuilder::CreateTextureUAV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto textureResource = std::dynamic_pointer_cast<RenderGraphTexture>(resource);
        if (!textureResource)
        {

        }

        RenderGraphTextureUAVDesc desc;
        desc.Texture = textureResource.get();

        return std::make_shared<RenderGraphTextureUAV>(name,desc);
    }

    RenderGraphBufferUAVSP RenderGraphBuilder::CreateBufferUAV(const std::string& name, RenderGraphResourceSP resource)
    {
        auto bufferResource = std::dynamic_pointer_cast<RenderGraphBuffer>(resource);
        if (!bufferResource)
        {
            
        }

        RenderGraphBufferUAVDesc desc;
        desc.Buffer = bufferResource.get();

        return std::make_shared<RenderGraphBufferUAV>(name, desc);
    }
    RenderGraphTextureSP RenderGraphBuilder::RegisterExternalTexture(const std::string& name, RHITexture* texture)
    {
        auto it = ExternalTextureCache.find(name);
        if (it != ExternalTextureCache.end())
        {
            return it->second; // Return the cached texture
        }
        RenderGraphTextureDesc desc;
        (RHI::RHITextureDesc)desc = texture->GetDesc();
        auto textureResource = std::make_shared<RenderGraphTexture>(name, desc);
        textureResource->SetRHITexture(texture);
        ExternalTextureCache[name] = textureResource;
        return ExternalTextureCache[name];
    }
    RenderGraphTextureSP RenderGraphBuilder::GetExternalTexture(const std::string& name)
    {
        return ExternalTextureCache[name];
    }
    RHITexture* RenderGraphBuilder::GetTexture(RenderGraphResourceSP resource)
    {
        auto textureResource = std::dynamic_pointer_cast<RenderGraphTexture>(resource);
        if (!textureResource)
        {
        }
        return textureResource->GetRHITexture();
    }

    RHIBuffer* RenderGraphBuilder::GetBuffer(RenderGraphResourceSP resource)
    {
        auto bufferResource = std::dynamic_pointer_cast<RenderGraphBuffer>(resource);
        if (!bufferResource)
        {

        }
        return bufferResource->GetRHIBuffer();
    }

    RenderGraphTextureSP RenderGraphBuilder::GetTexture(const std::string& name)
    {
        return TextureCache[name];
    }

    RenderGraphBufferSP RenderGraphBuilder::GetBuffer(const std::string& name)
    {
        return BufferCache[name];
    }

    void RenderGraphBuilder::Execute()
    {

        // use Common::TaskPool from Common/TaskPool.h
        Core::TaskPool taskPool(std::thread::hardware_concurrency());

        std::vector<Core::TaskHandle> handles;
        handles.reserve(ParallelPasses.size());
        std::vector<RHI::RHICommandListBase*> RecordedCommansLists;
		RecordedCommansLists.reserve(ParallelPasses.size());
        // For each parallel group, create one task that creates a command context/list
        // and executes each pass in the group sequentially: begin barriers, pass work, end barriers.
        for (const auto& group : ParallelPasses)
        {
            // capture a copy of the group
            std::vector<RenderGraphPass*> groupCopy;
            groupCopy.reserve(group.size());
            for (auto* p : group) groupCopy.push_back(p);

            // create a command context from global RHI
            RHI::RHIApi* api = GRHIApi;
            if (!api) return;

            RHI::RHIQueue* queue = api->GetQueue(RHI::EQueueType::Graphics);
            if (!queue) return;

            RHI::RHIContextBase* ctx = queue->AcquireCommandContext();
            if (!ctx) return;

            auto* graphicContext = dynamic_cast<RHI::RHIGraphicContex*>(ctx);
            if (!graphicContext) return;

            RHI::RHICommandListBase* cmdListSP = new RHI::RHIGraphicCommandList(graphicContext);
            RecordedCommansLists.push_back(cmdListSP);

            auto handle = taskPool.AddTask([groupCopy = std::move(groupCopy), cmdListSP]() {


                RHI::RHICommandListBase& cmdList = *cmdListSP;

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

        // merge commandlist


        // enqueue command
        EnqueueRenderCommand("Execute Render Graph Builder", [RecordedCommansLists](RHI::RHICommandListBase& commandList) {
            for (auto cmdList : RecordedCommansLists) {
                if (cmdList) {
                    commandList.Merge(*cmdList);
                }
            }
        });
    }

    void RenderGraphBuilder::AnalyzePasses()
    {
        // 追踪每个资源的当前状态
        struct LocalResState {
            RenderGraphPass* LastVisitor = nullptr;
            ERHIResourceAccess LastAccess = ERHIResourceAccess::Unknown;
        };
        std::unordered_map<RenderGraphResource*, LocalResState> ResourceStates;

        // --- 第一阶段：遍历所有 Pass，收集 Intent 并建立依赖/屏障 ---
        for (auto* Pass : Passes)
        {
            // 1. 处理纹理意图 (TextureIntents)
            for (const auto& Intent : Pass->TextureIntents)
            {
                auto& State = ResourceStates[Intent.Texture];

                // 如果之前有 Pass 访问过，且访问权限不兼容（或是为了确保状态切换）
                if (State.LastVisitor && State.LastVisitor != Pass)
                {
                    // 只有状态确实需要改变时才添加屏障（或者处理 Read-after-Write 等）
                    if (State.LastAccess != Intent.RequiredAccess)
                    {
                        RHI::RHITransitionInfo Transition{};
                        Transition.Type = RHI::RHITransitionInfo::EType::Texture;
                        Transition.Texture = Intent.Texture->GetRHITexture(); // 注意：这里可能需要延迟到真正的执行时刻获取
                        Transition.AccessBefore = State.LastAccess;
                        Transition.AccessAfter = Intent.RequiredAccess;
                        Transition.MipIndex = Intent.SubresourceRange.MipIndex;
                        Transition.ArraySlice = Intent.SubresourceRange.ArraySlice;
                        Transition.PlaneSlice = Intent.SubresourceRange.PlaneSlice;

                        // 核心修改：将屏障放在当前 Pass 执行之前
                        Pass->BeginBarrier.AddTransition(Transition);
                    }

                    // 建立 DAG 依赖
                    State.LastVisitor->PassConsumers.push_back(Pass);
                    Pass->PassProducers.push_back(State.LastVisitor);
                }

                // 更新资源最后访问状态
                State.LastVisitor = Pass;
                State.LastAccess = Intent.RequiredAccess;
            }

            // 2. 处理 Buffer 意图 (BufferStates)
            for (const auto& Intent : Pass->BufferStates)
            {
                auto& State = ResourceStates[Intent.Buffer];

                if (State.LastVisitor && State.LastVisitor != Pass)
                {
                    if (State.LastAccess != Intent.RequiredAccess)
                    {
                        RHI::RHITransitionInfo Transition{};
                        Transition.Type = RHI::RHITransitionInfo::EType::Buffer;
                        Transition.Buffer = Intent.Buffer->GetRHIBuffer();
                        Transition.AccessBefore = State.LastAccess;
                        Transition.AccessAfter = Intent.RequiredAccess;

                        Pass->BeginBarrier.AddTransition(Transition);
                    }

                    State.LastVisitor->PassConsumers.push_back(Pass);
                    Pass->PassProducers.push_back(State.LastVisitor);
                }

                State.LastVisitor = Pass;
                State.LastAccess = Intent.RequiredAccess;
            }
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

    void RenderGraphBuilder::SetupPassInternal(RenderGraphPass* Pass, const ShaderParametersMetadata* Metadata, const void* Parameters)
    {
        const uint8_t* BaseDataPtr = reinterpret_cast<const uint8_t*>(Parameters);

        // 遍历所有成员，寻找资源类型
        for (const auto& Member : Metadata->GetMembers())
        {
            // 计算成员在结构体中的实际地址
            const uint8_t* MemberAddr = BaseDataPtr + Member.Offset;

            if (Member.IsResource())
            {
                // 1. 处理纹理相关 (Texture, Texture_UAV)
                if (Member.BaseType == EShaderUniformBaseType::Texture ||
                    Member.BaseType == EShaderUniformBaseType::Texture_UAV)
                {
                    // 这里的关键：直接将内存解析为 RenderGraphTexture* 指针
                    // 注意：如果你的参数宏支持的是 RDG_TEXTURE_UAV 等包装类，这里需要对应调整
                    RenderGraphTexture* Tex = *reinterpret_cast<RenderGraphTexture* const*>(MemberAddr);

                    if (Tex)
                    {
                        RenderGraphPass::RenderGraphTextureIntent Intent;
                        Intent.Texture = Tex;
                        Intent.SubresourceRange = RHISubresourceRange(); // 默认全资源访问

                        // 根据元数据类型决定 RHI 访问权限
                        Intent.RequiredAccess = (Member.BaseType == EShaderUniformBaseType::Texture_UAV)
                            ? ERHIResourceAccess::UAVGraphics
                            : ERHIResourceAccess::UAVGraphics;

                        // 【核心】因为是友元，直接 push 到 Pass 的私有 vector 中
                        Pass->TextureIntents.push_back(Intent);


                    }
                }
                // 2. 处理 Buffer 相关 (Buffer, Buffer_UAV)
                else if (Member.BaseType == EShaderUniformBaseType::Buffer ||
                    Member.BaseType == EShaderUniformBaseType::Buffer_UAV)
                {
                    RenderGraphBuffer* Buf = *reinterpret_cast<RenderGraphBuffer* const*>(MemberAddr);

                    if (Buf)
                    {
                        RenderGraphPass::RenderGraphBufferIntent Intent;
                        Intent.Buffer = Buf;
                        Intent.Offset = 0;
                        Intent.Size = 0; // 全缓冲
                        Intent.RequiredAccess = (Member.BaseType == EShaderUniformBaseType::Buffer_UAV)
                            ? ERHIResourceAccess::UAVGraphics
                            : ERHIResourceAccess::UAVGraphics;

                        // 【核心】直接操作私有成员 BufferStates
                        Pass->BufferStates.push_back(Intent);

                    }
                }
            }
            // 3. 处理嵌套结构体 (如果你的 Metadata 支持嵌套)
            else if (Member.IsStruct())
            {
                SetupPassInternal(Pass, Member.StructMetadata, MemberAddr);
            }
        }
    }


} // namespace RenderCore
