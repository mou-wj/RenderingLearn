#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include "RenderGraphResource.h"
#include "TaskPool.h"
#include "RenderThread.h"
#include "RHICommandContex.h"
#include <unordered_set>
#include <queue>
using namespace RHI;

namespace RenderCore {


    RenderGraphBuilder::RenderGraphBuilder() : TransientAllocator(GTransientResourceAllocator)
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

    RenderGraphTextureRef RenderGraphBuilder::CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc)
    {
        auto it = TextureCache.find(name);
        if (it != TextureCache.end())
        {
            return it->second;
        }

        // 用 Allocator
        auto texture = Allocator.Allocate<RenderGraphTexture>(name, desc);

        TextureCache[name] = texture;
        return texture;
    }

    RenderGraphBufferRef RenderGraphBuilder::CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc)
    {
        auto it = BufferCache.find(name);
        if (it != BufferCache.end())
        {
            return it->second;
        }

        auto buffer = Allocator.Allocate<RenderGraphBuffer>(name, desc);


        BufferCache[name] = buffer;
        return buffer;
    }

    RenderGraphTextureSRVRef RenderGraphBuilder::CreateTextureSRV(const std::string& name, const RenderGraphTextureSRVDesc& desc)
    {
        auto it = TextureSRVCache.find(name);
        if (it != TextureSRVCache.end())
        {
            return it->second;
        }

        auto srv = Allocator.Allocate<RenderGraphTextureSRV>(name, desc);

        TextureSRVCache[name] = srv;
        return srv;
    }
    
    RenderGraphBufferSRVRef RenderGraphBuilder::CreateBufferSRV(const std::string& name, const RenderGraphBufferSRVDesc& desc)
    {
        auto it = BufferSRVCache.find(name);
        if (it != BufferSRVCache.end())
        {
            return it->second;
        }

        auto srv = Allocator.Allocate<RenderGraphBufferSRV>(name, desc);

        BufferSRVCache[name] = srv;
        return srv;
    }

    RenderGraphTextureUAVRef RenderGraphBuilder::CreateTextureUAV(const std::string& name, const RenderGraphTextureUAVDesc& desc)
    {
        auto it = TextureUAVCache.find(name);
        if (it != TextureUAVCache.end())
        {
            return it->second;
        }

        auto uav = Allocator.Allocate<RenderGraphTextureUAV>(name, desc);

        TextureUAVCache[name] = uav;
        return uav;
    }

    RenderGraphBufferUAVRef RenderGraphBuilder::CreateBufferUAV(const std::string& name, const RenderGraphBufferUAVDesc& desc)
    {
        auto it = BufferUAVCache.find(name);
        if (it != BufferUAVCache.end())
        {
            return it->second;
        }

        auto uav = Allocator.Allocate<RenderGraphBufferUAV>(name, desc);

        BufferUAVCache[name] = uav;
        return uav;
    }
    RenderGraphTextureRef RenderGraphBuilder::RegisterExternalTexture(const std::string& name, PooledRenderTarget* target)
    {
        auto it = ExternalTextureCache.find(name);
        if (it != ExternalTextureCache.end())
        {
            return it->second; // Return the cached texture
        }
        RenderGraphTextureDesc desc;
        (RHI::RHITextureDesc)desc = PoolRenderTargetDesc::ConvertToRHITextureDesc(target->GetDesc());
        auto textureResource = CreateTexture(name, desc);
        textureResource->SetRHITexture(target->GetRHI());
        ExternalTextureCache[name] = textureResource;
        PoolTarget2RDGTexture[target] =  ExternalTextureCache[name];
        return ExternalTextureCache[name];
    }
    RenderGraphTextureRef RenderGraphBuilder::GetExternalTexture(const std::string& name)
    {
        return ExternalTextureCache[name];
    }
    RHITexture* RenderGraphBuilder::GetTexture(RenderGraphResourceRef resource)
    {
        auto textureResource = static_cast<RenderGraphTexture*>(resource);
        if (!textureResource)
        {
            // handle error
        }
        return textureResource->GetRHITexture();
    }

    RHIBuffer* RenderGraphBuilder::GetBuffer(RenderGraphResourceRef resource)
    {
        auto bufferResource = static_cast<RenderGraphBuffer*>(resource);
        if (!bufferResource)
        {
            // handle error
        }
        return bufferResource->GetRHIBuffer();
    }

    RenderGraphTextureRef RenderGraphBuilder::GetTexture(const std::string& name)
    {
        return TextureCache[name];
    }

    RenderGraphBufferRef RenderGraphBuilder::GetBuffer(const std::string& name)
    {
        return BufferCache[name];
    }

    void RenderGraphBuilder::Execute()
    {

        Core::TaskPool taskPool(std::thread::hardware_concurrency());

        struct RecordedGroup
        {
            std::unordered_map<RHI::EQueueType, RHI::RHICommandListBase*> CmdLists;
            std::unordered_map<RHI::EQueueType, std::vector<RHI::RHITransitionInfo>> BeginBarriers;
            std::unordered_map<RHI::EQueueType, RHI::RHIContextBase*> CmdContexts;
        };

        std::vector<RecordedGroup> RecordedGroups(ParallelPasses.size());
        std::vector<Core::TaskHandle> handles;

        RHI::RHIApi* api = GRHIApi;
        if (!api) return;

        // =========================
        // 1️⃣ 并行录制
        // =========================
        int i = 0;
        for (auto iter = ParallelPasses.begin(); iter != ParallelPasses.end(); ++iter,++i)
        {
            auto group = *iter;

            handles.push_back(taskPool.AddTask([this, api, group, &RecordedGroups, i]()
                {
                    auto& outGroup = RecordedGroups[i];
                    auto& cmdLists = outGroup.CmdLists;
					auto& cmdContexts = outGroup.CmdContexts;

                    auto getCmd = [&](RHI::EQueueType q)
                        {
                            if (cmdLists.count(q)) return cmdLists[q];

                            auto* queue = api->GetQueue(q);
                            auto* ctx = queue->AcquireCommandContext();

                            RHI::RHICommandListBase* cmd = nullptr;

                            if (q == RHI::EQueueType::Graphics)
                                cmd = Allocator.Allocate<RHI::RHIGraphicCommandList>(dynamic_cast<RHI::RHIGraphicContex*>(ctx));
                            else if (q == RHI::EQueueType::Compute)
                                cmd = Allocator.Allocate <RHI::RHIComputeCommandList>(dynamic_cast<RHI::RHIComputeContex*>(ctx));
                            else
                                cmd = Allocator.Allocate<RHI::RHITransferCommandList>(dynamic_cast<RHI::RHITransferContext*>(ctx));

                            cmd->Begin();
                            cmdLists[q] = cmd;
                            cmdContexts[q] = ctx;
                            return cmd;
                        };

                    // 👉 收集 barrier
                    for (auto* pass : group)
                    {
                        auto q = (RHI::EQueueType)pass->GetPassFlag();

                        for (auto& t : pass->BeginBarrier.GetTransitions())
                        {
                            outGroup.BeginBarriers[q].push_back(t);
                        }
                    }

                    // 👉 执行 barrier（合并）
                    for (auto& [q, transitions] : outGroup.BeginBarriers)
                    {
                        auto* cmd = getCmd(q);

                        if (!transitions.empty())
                        {
							void* mem = Allocator.AllocateBytes(RHI::G_RHITransition_TotalSize);
							RHI::RHITransition* transitionObj = new(mem) RHI::RHITransition();
							RHI::RHITransitionCreateInfo createInfo;
							createInfo.Flags = RHI::ERHITransitionCreateFlags::None;
                            createInfo.TransitionInfos = transitions;
                            RHI::GRHIApi->RHICreateTransition(transitionObj, createInfo);
                            cmd->BeginTransitions({ transitionObj });
                            cmd->EndTransitions({ transitionObj });
                        }
                    }

                    // 👉 执行 pass
                    for (auto* pass : group)
                    {
                        auto q = (RHI::EQueueType)pass->GetPassFlag();
                        auto* cmd = getCmd(q);

                        pass->Execute(*cmd);
                    }

                    for (auto& [_, cmd] : cmdLists)
                    {
                        cmd->End();
                    }
                }));
        }

        taskPool.WaitAll(handles);

        // =========================
        // 2️⃣ 顺序提交（带必要同步）
        // =========================
        std::unordered_map<RHI::EQueueType, uint64_t> Timeline;

        for (int i = 0; i < RecordedGroups.size(); ++i)
        {
            auto& group = RecordedGroups[i];

            for (auto& [q, cmd] : group.CmdLists)
            {
                auto* queue = api->GetQueue(q);
				std::vector<RHI::RHIWaitInfo> waitInfos;
                for (auto& [otherQ, value] : Timeline)
                {
                    if (otherQ == q) continue;

                    RHI::RHIWaitInfo waitInfo;
                    waitInfo.QueueType = otherQ;
                    waitInfo.Value = value;
                    waitInfo.WaitStage = RHI::ERHIPipelineStage::AllCommands;

                    waitInfos.push_back(waitInfo);
                }

				RHI::RHIWaitInfo lastTransInfo;
                RHI::RHIFence fence = queue->ExecuteContext({ group.CmdContexts[q] }, waitInfos);
                Timeline[q] = fence.Value;
            }

            // 👉 TODO：只根据依赖插入 wait（你后面可以加）
        }
		Allocator.Reset();
        ApplyFinalStates();
    }

    void RenderGraphBuilder::AnalyzePasses()
    {
        // =========================================
            // 清理旧数据
            // =========================================
        ParallelPasses.clear();
        InitialStates.clear();
        FinalStates.clear();


        // =========================================
        // 工具函数
        // =========================================

        auto IsWrite = [](ERHIResourceAccess A)
            {
                return EnumHasAnyFlags(A, ERHIResourceAccess::WritableMask);
            };

        auto IsUAV = [](ERHIResourceAccess A)
            {
                return EnumHasAnyFlags(A, ERHIResourceAccess::UAVMask);
            };

        auto NeedBarrier = [&](ERHIResourceAccess Before, ERHIResourceAccess After)
            {
                const bool BeforeWrite = IsWrite(Before);
                const bool AfterWrite = IsWrite(After);

                if (IsUAV(Before) && IsUAV(After))
                    return true;

                if (!BeforeWrite && !AfterWrite)
                    return false;

                return Before != After;
            };

        // =========================================
        // Runtime tracking
        // =========================================
        struct LocalResState
        {
            RenderGraphPass* LastVisitor = nullptr;
            ERHIResourceAccess LastAccess = ERHIResourceAccess::Unknown;
            bool bInitialized = false;
        };

        std::unordered_map<ResourceSubresourceKey, LocalResState, KeyHasher> ResourceStates;

        // =========================================
        // Pass index（👉 用于 lifetime）
        // =========================================
        std::unordered_map<RenderGraphPass*, uint32_t> PassIndex;
        uint32_t passOrder = 0;

        for (auto* p : Passes)
        {
            PassIndex[p] = passOrder++;
        }

        // =========================================
        // 获取初始状态
        // =========================================
        auto GetInitialAccess = [&](const ResourceSubresourceKey& Key)
            -> ERHIResourceAccess
            {
                auto it = InitialStates.find(Key);
                if (it != InitialStates.end())
                    return it->second;

                ERHIResourceAccess access = ERHIResourceAccess::Undefined;

                InitialStates[Key] = access;
                return access;
            };

        // =========================================
        // Phase 1: Barrier + DAG + Lifetime tracking
        // =========================================
        for (auto* Pass : Passes)
        {
            // =========================
            // Texture
            // =========================
            for (const auto& Intent : Pass->TextureIntents)
            {
                ResourceSubresourceKey Key{
                    Intent.Texture,
                    Intent.SubresourceRange,
                    false
                };

                auto& State = ResourceStates[Key];

                if (!State.bInitialized)
                {
                    State.LastAccess = GetInitialAccess(Key);
                    State.bInitialized = true;
                }

                // =========================
                // 👉 Lifetime tracking（核心新增）
                // =========================
                {
                    auto* Res = Intent.Texture;
                    auto& LT = ResourceLifetimes[Res];

                    if (!LT.FirstPass)
                        LT.FirstPass = Pass;

                    LT.LastPass = Pass;

                    uint32_t idx = PassIndex[Pass];
                    LT.BeginPassIndex = std::min(LT.BeginPassIndex, idx);
                    LT.EndPassIndex = std::max(LT.EndPassIndex, idx);
                }

                // =========================
                // Barrier + DAG
                // =========================
                if (State.LastVisitor && State.LastVisitor != Pass)
                {
                    if (NeedBarrier(State.LastAccess, Intent.RequiredAccess))
                    {
                        RHI::RHITransitionInfo Transition{};
                        Transition.Type = RHI::RHITransitionInfo::EType::Texture;
                        Transition.Texture = Intent.Texture->GetRHITexture();
                        Transition.AccessBefore = State.LastAccess;
                        Transition.AccessAfter = Intent.RequiredAccess;

                        Pass->BeginBarrier.AddTransition(Transition);

                        State.LastVisitor->PassConsumers.push_back(Pass);
                        Pass->PassProducers.push_back(State.LastVisitor);
                    }
                }

                State.LastVisitor = Pass;
                State.LastAccess = Intent.RequiredAccess;

                FinalStates[Key] = Intent.RequiredAccess;
            }

            // =========================
            // Buffer
            // =========================
            for (const auto& Intent : Pass->BufferStates)
            {
                ResourceSubresourceKey Key{
                    Intent.Buffer,
                    RHI::RHISubresourceRange(),
                    true,
                    Intent.Offset,
                    Intent.Size
                };

                auto& State = ResourceStates[Key];

                if (!State.bInitialized)
                {
                    State.LastAccess = GetInitialAccess(Key);
                    State.bInitialized = true;
                }

                // =========================
                // 👉 Lifetime tracking（核心新增）
                // =========================
                {
                    auto* Res = Intent.Buffer;
                    auto& LT = ResourceLifetimes[Res];

                    if (!LT.FirstPass)
                        LT.FirstPass = Pass;

                    LT.LastPass = Pass;

                    uint32_t idx = PassIndex[Pass];
                    LT.BeginPassIndex = std::min(LT.BeginPassIndex, idx);
                    LT.EndPassIndex = std::max(LT.EndPassIndex, idx);
                }

                // =========================
                // Barrier + DAG
                // =========================
                if (State.LastVisitor && State.LastVisitor != Pass)
                {
                    if (NeedBarrier(State.LastAccess, Intent.RequiredAccess))
                    {
                        RHI::RHITransitionInfo Transition{};
                        Transition.Type = RHI::RHITransitionInfo::EType::Buffer;
                        Transition.Buffer = Intent.Buffer->GetRHIBuffer();
                        Transition.AccessBefore = State.LastAccess;
                        Transition.AccessAfter = Intent.RequiredAccess;

                        Pass->BeginBarrier.AddTransition(Transition);

                        State.LastVisitor->PassConsumers.push_back(Pass);
                        Pass->PassProducers.push_back(State.LastVisitor);
                    }
                }

                State.LastVisitor = Pass;
                State.LastAccess = Intent.RequiredAccess;

                FinalStates[Key] = Intent.RequiredAccess;
            }
        }

        // =========================================
        // Phase 2: 拓扑排序（不影响 lifetime）
        // =========================================
        std::unordered_map<RenderGraphPass*, size_t> indegree;
        std::unordered_set<RenderGraphPass*> allNodes;

        for (auto* pass : Passes)
        {
            allNodes.insert(pass);
            indegree[pass] = pass->PassProducers.size();
        }

        std::queue<RenderGraphPass*> zeroQueue;

        for (auto* node : allNodes)
        {
            if (indegree[node] == 0)
                zeroQueue.push(node);
        }

        std::unordered_set<RenderGraphPass*> processed;

        while (!allNodes.empty())
        {
            std::vector<RenderGraphPass*> currentSet;

            if (zeroQueue.empty())
            {
                auto it = allNodes.begin();
                currentSet.push_back(*it);
            }
            else
            {
                size_t count = zeroQueue.size();
                for (size_t i = 0; i < count; ++i)
                {
                    auto* n = zeroQueue.front();
                    zeroQueue.pop();

                    if (!processed.count(n))
                        currentSet.push_back(n);
                }
            }

            PassList group;

            for (auto* node : currentSet)
            {
                group.push_back(node);
                processed.insert(node);
                allNodes.erase(node);
            }

            for (auto* node : currentSet)
            {
                for (auto* consumer : node->PassConsumers)
                {
                    if (--indegree[consumer] == 0)
                        zeroQueue.push(consumer);
                }
            }

            if (!group.empty())
                ParallelPasses.push_back(group);
        }

        // fallback
        for (auto* pass : Passes)
        {
            if (!processed.count(pass))
            {
                PassList single;
                single.push_back(pass);
                ParallelPasses.push_back(single);
            }
        }

        // =========================================
        // 👉 （可选）这里可以做 lifetime finalize
        // =========================================
        // ResourceLifetimes 就已经完整了：
        //
        // BeginPassIndex / EndPassIndex
        // FirstPass / LastPass
    }

    void RenderGraphBuilder::AllocateResources()
    {
        // =========================
            // Texture
            // =========================
        for (auto& [name, tex] : TextureCache)
        {
            if (!tex) continue;

            // 外部资源跳过
            if (tex->IsExternal)
                continue;

            // 已创建跳过
            if (tex->GetRHITexture())
                continue;

            // =========================================
            // 👉 关键改动：读取 lifetime
            // =========================================
            auto it = ResourceLifetimes.find(tex);
            if (it == ResourceLifetimes.end())
                continue; // 没有被 graph 使用的资源不创建

            const ResourceLifetime& LT = it->second;

            // =========================================
            // 创建 transient texture（带生命周期）
            // =========================================
            PoolRenderTargetDesc Desc =
                PoolRenderTargetDesc::ConvertFromRHITextureDesc(tex->GetDesc());

            auto rhiTex = TransientAllocator->AllocateRenderTarget(
                Desc,
                LT.BeginPassIndex,
                LT.EndPassIndex
            );

            tex->SetRHITexture(rhiTex->GetRHI());
        }

        // =========================
        // Buffer
        // =========================
        for (auto& [name, buf] : BufferCache)
        {
            if (!buf) continue;

            if (buf->IsExternal)
                continue;

            if (buf->GetRHIBuffer())
                continue;

            // =========================================
            // 👉 lifetime driven
            // =========================================
            auto it = ResourceLifetimes.find(buf);
            if (it == ResourceLifetimes.end())
                continue;

            const ResourceLifetime& LT = it->second;

            auto rhiBuf = TransientAllocator->AllocateBuffer(
                RenderGraphBufferDesc::ConvertToRHIDesc(buf->GetDesc()),
                LT.BeginPassIndex,
                LT.EndPassIndex
            );

            buf->SetRHIBuffer(rhiBuf->GetRHI());
        }
    }

    void RenderGraphBuilder::SetupPassInternal(
        RenderGraphPass* Pass,
        const ShaderParametersMetadata* Metadata,
        const void* Parameters)
    {
        const uint8_t* BaseDataPtr = reinterpret_cast<const uint8_t*>(Parameters);

        for (const auto& Member : Metadata->GetMembers())
        {
            const uint8_t* MemberAddr = BaseDataPtr + Member.Offset;

            if (Member.IsResource())
            {
                // ============================
                // Texture SRV
                // ============================
                if (Member.BaseType == EShaderUniformBaseType::Texture_SRV)
                {
                    auto SRV = *reinterpret_cast<RenderGraphTextureSRV* const*>(MemberAddr);

                    if (SRV)
                    {
                        auto* Tex = static_cast<RenderGraphTexture*>(SRV->GetResource());
                        const auto& Desc = SRV->GetDesc();

                        RenderGraphPass::RenderGraphTextureIntent Intent;
                        Intent.Texture = Tex;

                        Intent.SubresourceRange = RHISubresourceRange(
                            Desc.MipLevel,
                            Desc.ArraySlice,
                            RHISubresourceRange::kAllSubresources
                        );

                        // SRV → ReadOnly
                        Intent.RequiredAccess =
                            (Pass->GetPassFlag() == EPassFlag::Compute)
                            ? ERHIResourceAccess::SRVCompute
                            : ERHIResourceAccess::SRVGraphics;

                        Pass->TextureIntents.push_back(Intent);
                    }
                }
                // ============================
                // Texture UAV
                // ============================
                else if (Member.BaseType == EShaderUniformBaseType::Texture_UAV)
                {
                    auto UAV = *reinterpret_cast<RenderGraphTextureUAV* const*>(MemberAddr);

                    if (UAV)
                    {
                        auto* Tex = static_cast<RenderGraphTexture*>(UAV->GetResource());
                        const auto& Desc = UAV->GetDesc();

                        RenderGraphPass::RenderGraphTextureIntent Intent;
                        Intent.Texture = Tex;

                        Intent.SubresourceRange = RHISubresourceRange(
                            Desc.MipLevel,
                            Desc.ArraySlice,
                            RHISubresourceRange::kAllSubresources
                        );

                        // UAV → ReadWrite
                        Intent.RequiredAccess =
                            (Pass->GetPassFlag() == EPassFlag::Compute)
                            ? ERHIResourceAccess::UAVCompute
                            : ERHIResourceAccess::UAVGraphics;

                        Pass->TextureIntents.push_back(Intent);
                    }
                }
                // ============================
                // Texture (裸Texture，极少用)
                // ============================
                else if (Member.BaseType == EShaderUniformBaseType::Texture)
                {
                    auto Tex = *reinterpret_cast<RenderGraphTexture* const*>(MemberAddr);

                    if (Tex)
                    {
                        RenderGraphPass::RenderGraphTextureIntent Intent;
                        Intent.Texture = Tex;
                        Intent.SubresourceRange = RHISubresourceRange(); // whole

                        Intent.RequiredAccess =
                            (Pass->GetPassFlag() == EPassFlag::Compute)
                            ? ERHIResourceAccess::SRVCompute
                            : ERHIResourceAccess::SRVGraphics;

                        Pass->TextureIntents.push_back(Intent);
                    }
                }

                // ============================
                // Buffer SRV
                // ============================
                else if (Member.BaseType == EShaderUniformBaseType::Buffer_SRV)
                {
                    auto SRV = *reinterpret_cast<RenderGraphBufferSRV* const*>(MemberAddr);
                    auto vDesc = SRV->GetDesc();
                    if (SRV)
                    {
                        auto* Buf = static_cast<RenderGraphBuffer*>(SRV->GetResource());

                        RenderGraphPass::RenderGraphBufferIntent Intent;
                        Intent.Buffer = Buf;

                        // Buffer SRV 默认 whole buffer
                        Intent.Offset = vDesc.Offset;
                        Intent.Size = vDesc.Size;

                        Intent.RequiredAccess =
                            (Pass->GetPassFlag() == EPassFlag::Compute)
                            ? ERHIResourceAccess::SRVCompute
                            : ERHIResourceAccess::SRVGraphics;

                        Pass->BufferStates.push_back(Intent);
                    }
                }

                // ============================
                // Buffer UAV
                // ============================
                else if (Member.BaseType == EShaderUniformBaseType::Buffer_UAV)
                {
                    auto UAV = *reinterpret_cast<RenderGraphBufferUAV* const*>(MemberAddr);
					auto uavDesc = UAV->GetDesc();
                    if (UAV)
                    {
                        auto* Buf = static_cast<RenderGraphBuffer*>(UAV->GetResource());

                        RenderGraphPass::RenderGraphBufferIntent Intent;
                        Intent.Buffer = Buf;

                        Intent.Offset = uavDesc.Offset;
                        Intent.Size = uavDesc.Size;

                        Intent.RequiredAccess =
                            (Pass->GetPassFlag() == EPassFlag::Compute)
                            ? ERHIResourceAccess::UAVCompute
                            : ERHIResourceAccess::UAVGraphics;

                        Pass->BufferStates.push_back(Intent);
                    }
                }

                // ============================
                // Buffer（裸）
                // ============================
                else if (Member.BaseType == EShaderUniformBaseType::Buffer)
                {
                    auto Buf = *reinterpret_cast<RenderGraphBuffer* const*>(MemberAddr);

                    if (Buf)
                    {
                        RenderGraphPass::RenderGraphBufferIntent Intent;
                        Intent.Buffer = Buf;
                        Intent.Offset = 0;
                        Intent.Size = 0;

                        Intent.RequiredAccess =
                            (Pass->GetPassFlag() == EPassFlag::Compute)
                            ? ERHIResourceAccess::SRVCompute
                            : ERHIResourceAccess::SRVGraphics;

                        Pass->BufferStates.push_back(Intent);
                    }
                }
            }
            else if (Member.IsStruct())
            {
                SetupPassInternal(Pass, Member.StructMetadata, MemberAddr);
            }
        }
    }

    void RenderGraphBuilder::ApplyFinalStates()
    {
        for (auto& [Key, Access] : FinalStates)
        {
            if (!Key.Resource)
                continue;



            // =========================
            // Texture
            // =========================
            if (!Key.isBuffer)
            {
                auto* tex = static_cast<RenderGraphTexture*>(Key.Resource);
                if (!tex->IsExternal)
                    continue;
                auto it = RDGTexture2PoolTarget.find(tex);
                if (it == RDGTexture2PoolTarget.end())
                    continue;

                PooledRenderTarget* pool = it->second;
                if (!pool)
                    continue;

                // 👉 写回 subresource state
                pool->GetTracker().UpdateSubresourceAccess(Key.Range, Access);
            }
            // =========================
            // Buffer
            // =========================
            else
            {
                auto* buf = static_cast<RenderGraphBuffer*>(Key.Resource);
                if (!buf->IsExternal)
                    continue;
                // 如果以后有 external buffer，再补映射
                // 当前可以忽略或留 TODO

                // 示例：
                // ExternalBufferState[buf->GetRHIBuffer()] = Access;
            }
        }
    }


} // namespace RenderCore
