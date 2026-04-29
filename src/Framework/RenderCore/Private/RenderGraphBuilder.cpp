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
        if (!target)
            return nullptr;

        RHI::RHITexture* rhiTex = target->GetRHI();

        // 👉 已注册（按 RHI 去重）
        auto it = ExternalTextures.find(rhiTex);
        if (it != ExternalTextures.end())
        {
            return it->second.RDGTexture;
        }

        // 👉 创建 RDGTexture
        RenderGraphTextureDesc desc;
        desc = (RenderGraphTextureDesc)PoolRenderTargetDesc::ConvertToRHITextureDesc(target->GetDesc());

        auto rdgTex = CreateTexture(name, desc);

        rdgTex->SetRHITexture(rhiTex);
        rdgTex->IsExternal = true;

        // 👉 建立映射
        RDGToRHITexture[rdgTex] = rhiTex;

        ExternalTextureEntry entry;
        entry.RDGTexture = rdgTex;
        entry.Tracker = &target->GetTracker();
        entry.ViewCache = &target->GetViewCache();
        entry.RHITexture = rhiTex;

        ExternalTextures[rhiTex] = entry;

        return rdgTex;
    }
    RenderGraphTextureRef RenderGraphBuilder::RegisterExternalTexture(const std::string& name, RenderTexture* texture)
    {
        if (!texture)
            return nullptr;

        RHI::RHITexture* rhiTex = texture->GetRHI();

        auto it = ExternalTextures.find(rhiTex);
        if (it != ExternalTextures.end())
        {
            return it->second.RDGTexture;
        }

        // 👉 构造 desc（你可能需要补一个转换函数）
        RenderGraphTextureDesc desc;
        static_cast<RHI::RHITextureDesc>(desc) = texture->GetRHI()->GetDesc(); // ⚠️ 如果没有，需要你自己实现转换

        auto rdgTex = CreateTexture(name, desc);

        rdgTex->SetRHITexture(rhiTex);
        rdgTex->IsExternal = true;

        RDGToRHITexture[rdgTex] = rhiTex;

        ExternalTextureEntry entry;
        entry.RDGTexture = rdgTex;
        entry.Tracker = &texture->GetTracker();
        entry.RHITexture = rhiTex;
        entry.ViewCache = &texture->GetViewCache();

        ExternalTextures[rhiTex] = entry;

        return rdgTex;
    }
    RenderGraphBufferRef RenderGraphBuilder::RegisterExternalBuffer(const std::string& name, RenderBuffer* buffer)
    {
        if (!buffer)
            return nullptr;

        RHI::RHIBuffer* rhiBuf = buffer->GetRHI();

        auto it = ExternalBuffers.find(rhiBuf);
        if (it != ExternalBuffers.end())
        {
            return it->second.RDGBuffer;
        }

        // 👉 构造 desc
        RenderGraphBufferDesc desc;
        static_cast<RHI::RHIBufferDesc>(desc) = buffer->GetRHI()->GetDesc(); // ⚠️ 同样需要转换

        auto rdgBuf = CreateBuffer(name, desc);

        rdgBuf->SetRHIBuffer(rhiBuf);
        rdgBuf->IsExternal = true;

        RDGToRHIBuffer[rdgBuf] = rhiBuf;

        ExternalBufferEntry entry;
        entry.RDGBuffer = rdgBuf;
        entry.Tracker = &buffer->GetTracker();
        entry.RHIBuffer = rhiBuf;

        ExternalBuffers[rhiBuf] = entry;

        return rdgBuf;
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
                        pass->BeginBarrier.Process();
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
        auto passGroupIter = ParallelPasses.begin();
        for (int i = 0; i < RecordedGroups.size(); ++i, passGroupIter++)
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

                // 👉 关键：更新 fence
                for (auto* pass : *passGroupIter)
                {
                    auto it = PassLastUseResources.find(pass);
                    if (it == PassLastUseResources.end())
                        continue;

                    for (auto* res : it->second)
                    {
                        if (auto* tex = dynamic_cast<RenderGraphTexture*>(res))
                        {
                            if (tex->GetRHITexture())
                            {
                                tex->GetTracker().UpdateLastAccessFence(fence);
                            }
                        }
                        else if (auto* buf = dynamic_cast<RenderGraphBuffer*>(res))
                        {
                            if (buf->GetRHIBuffer())
                            {
                                buf->GetTracker().UpdateLastAccessFence(fence);
                            }
                        }
                    }
                }
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
        InitialTextureStates.clear();
        FinalTextureStates.clear();
        InitialBufferStates.clear();
        FinalBufferStates.clear();
        ResourceLifetimes.clear();
        PassLastUseResources.clear();

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
        struct TextureState
        {
            RenderGraphPass* LastVisitor = nullptr;
            ERHIResourceAccess LastAccess = ERHIResourceAccess::Unknown;
            bool bInitialized = false;
        };

        struct BufferState
        {
            RenderGraphPass* LastVisitor = nullptr;
            ERHIResourceAccess LastAccess = ERHIResourceAccess::Unknown;
            bool bInitialized = false;
        };

        std::unordered_map<TextureKey, TextureState, TextureKeyHasher> TextureStates;
        std::unordered_map<BufferKey, BufferState, BufferKeyHasher>  BufferStates;

        // =========================================
        // Pass index（用于 lifetime）
        // =========================================
        std::unordered_map<RenderGraphPass*, uint32_t> PassIndex;
        uint32_t passOrder = 0;

        for (auto* p : Passes)
        {
            PassIndex[p] = passOrder++;
        }

        // =========================================
        // 初始状态获取
        // =========================================
        auto GetInitialTextureAccess = [&](const TextureKey& Key)
            {
                auto it = InitialTextureStates.find(Key);
                if (it != InitialTextureStates.end())
                    return it->second;

                InitialTextureStates[Key] = ERHIResourceAccess::Undefined;
                return ERHIResourceAccess::Undefined;
            };

        auto GetInitialBufferAccess = [&](const BufferKey& Key)
            {
                auto it = InitialBufferStates.find(Key);
                if (it != InitialBufferStates.end())
                    return it->second;

                InitialBufferStates[Key] = ERHIResourceAccess::Undefined;
                return ERHIResourceAccess::Undefined;
            };

        // =========================================
        // Phase 1: Barrier + DAG + Lifetime
        // =========================================
        for (auto* Pass : Passes)
        {
            // =========================
            // Texture
            // =========================
            for (const auto& Intent : Pass->TextureIntents)
            {
                TextureKey Key{ Intent.Texture, Intent.SubresourceRange };
                auto& State = TextureStates[Key];

                if (!State.bInitialized)
                {
                    State.LastAccess = GetInitialTextureAccess(Key);
                    State.bInitialized = true;
                }

                // ===== Lifetime =====
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

                // ===== Barrier =====
                if (State.LastVisitor && State.LastVisitor != Pass)
                {
                    if (NeedBarrier(State.LastAccess, Intent.RequiredAccess))
                    {
                        RHI::RHITransitionInfo Transition{};
                        Transition.Type = RHI::RHITransitionInfo::EType::Texture;
                        Transition.AccessBefore = State.LastAccess;
                        Transition.AccessAfter = Intent.RequiredAccess;

                        // ✅ RDG层信息
                        static_cast<RHI::RHISubresourceRange>(Transition) = Intent.SubresourceRange;

                        Pass->BeginBarrier.AddTransition(Intent.Texture, Transition);

                        State.LastVisitor->PassConsumers.push_back(Pass);
                        Pass->PassProducers.push_back(State.LastVisitor);
                    }
                }

                State.LastVisitor = Pass;
                State.LastAccess = Intent.RequiredAccess;

                FinalTextureStates[Key] = Intent.RequiredAccess;
            }

            // =========================
            // Buffer
            // =========================
            for (const auto& Intent : Pass->BufferStates)
            {
                BufferKey Key{ Intent.Buffer };
                auto& State = BufferStates[Key];

                if (!State.bInitialized)
                {
                    State.LastAccess = GetInitialBufferAccess(Key);
                    State.bInitialized = true;
                }

                // ===== Lifetime =====
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

                // ===== Barrier =====
                if (State.LastVisitor && State.LastVisitor != Pass)
                {
                    if (NeedBarrier(State.LastAccess, Intent.RequiredAccess))
                    {
                        RHI::RHITransitionInfo Transition{};
                        Transition.Type = RHI::RHITransitionInfo::EType::Buffer;
                        Transition.AccessBefore = State.LastAccess;
                        Transition.AccessAfter = Intent.RequiredAccess;

                        Pass->BeginBarrier.AddTransition(Intent.Buffer, Transition);

                        State.LastVisitor->PassConsumers.push_back(Pass);
                        Pass->PassProducers.push_back(State.LastVisitor);
                    }
                }

                State.LastVisitor = Pass;
                State.LastAccess = Intent.RequiredAccess;

                FinalBufferStates[Key] = Intent.RequiredAccess;
            }
        }

        // =========================================
        // Phase 2: 拓扑排序（并行分组）
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

        //last use resource
        for (auto& [res, lifetime] : ResourceLifetimes)
        {
            if (lifetime.LastPass)
            {
                PassLastUseResources[lifetime.LastPass].push_back(res);
            }
        }
    }

    void RenderGraphBuilder::AllocateResources()
    {
        std::unordered_map<RHI::RHITexture*, TextureViewCache*> InternalTextureViewCaches;
        std::unordered_map<RHI::RHIBuffer*, BufferViewCache*> InternalBufferViewCaches;
        // =========================
            // Texture
            // =========================
        for (auto& [name, tex] : TextureCache)
        {
            if (!tex) continue;

            // 外部资源跳过
            if (tex->IsExternal)
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
            InternalTextureViewCaches[rhiTex->GetRHI()] = &rhiTex->GetViewCache(); // 内部资源创建独立 view cache
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
                TransientBufferDesc::ConvertFromRHIBufferDesc(buf->GetDesc()),
                LT.BeginPassIndex,
                LT.EndPassIndex
            );
            InternalBufferViewCaches[rhiBuf->GetRHI()] = &rhiBuf->GetViewCache();
            buf->SetRHIBuffer(rhiBuf->GetRHI());
        }

        //view
            // =========================
            // Texture SRV
            // =========================
        for (auto& [name, srv] : TextureSRVCache)
        {
            if (!srv) continue;
            if (srv->RHIShaderResourceView) continue;
            RenderGraphTexture* parent = srv->Desc.Texture;
            RHI::RHITexture* parentRHI = parent->GetRHITexture();
            TextureViewCache* viewCache = nullptr;
            if (parent->IsExternal) {
                viewCache = InternalTextureViewCaches[parentRHI];
            }
            else {
                viewCache = ExternalTextures[parentRHI].ViewCache;
            }
            RHI::RHITexSRVCreateInfo info = static_cast<RHI::RHITexSRVCreateInfo>(srv->Desc);
            auto rhi = viewCache->GetOrCreateSRV(parentRHI, info);
            srv->RHIShaderResourceView = rhi;
        }

        // =========================
        // Texture UAV
        // =========================
        for (auto& [name, uav] : TextureUAVCache)
        {
            if (!uav) continue;
            if (uav->RHIUnorderedAccessView) continue;
            RenderGraphTexture* parent = uav->Desc.Texture;
            RHI::RHITexture* parentRHI = parent->GetRHITexture();
            TextureViewCache* viewCache = nullptr;
            if (parent->IsExternal) {
                viewCache = InternalTextureViewCaches[parentRHI];
            }
            else {
                viewCache = ExternalTextures[parentRHI].ViewCache;
            }
            RHI::RHITexUAVCreateInfo info = static_cast<RHI::RHITexUAVCreateInfo>(uav->Desc);
            auto rhi = viewCache->GetOrCreateUAV(parentRHI, info);
            uav->RHIUnorderedAccessView = rhi;
        }

        // =========================
        // Buffer SRV
        // =========================
        for (auto& [name, srv] : BufferSRVCache)
        {
            if (!srv) continue;
            if (srv->RHIShaderResourceView) continue;
            RenderGraphBuffer* parent = srv->Desc.Buffer;
            RHI::RHIBuffer* parentRHI = parent->GetRHIBuffer();
            BufferViewCache* viewCache = nullptr;
            if (parent->IsExternal) {
                viewCache = InternalBufferViewCaches[parentRHI];
            }
            else {
                viewCache = ExternalBuffers[parentRHI].ViewCache;
            }
            RHI::RHIBufferSRVCreateInfo info = static_cast<RHI::RHIBufferSRVCreateInfo>(srv->Desc);
            auto rhi = viewCache->GetOrCreateSRV(parentRHI, info);
            srv->RHIShaderResourceView = rhi;
        }

        // =========================
        // Buffer UAV
        // =========================
        for (auto& [name, uav] : BufferUAVCache)
        {
            if (!uav) continue;
            if (uav->RHIUnorderedAccessView) continue;
            RenderGraphBuffer* parent = uav->Desc.Buffer;
            RHI::RHIBuffer* parentRHI = parent->GetRHIBuffer();
            BufferViewCache* viewCache = nullptr;
            if (parent->IsExternal) {
                viewCache = InternalBufferViewCaches[parentRHI];
            }
            else {
                viewCache = ExternalBuffers[parentRHI].ViewCache;
            }
            RHI::RHIBufferUAVCreateInfo info = static_cast<RHI::RHIBufferUAVCreateInfo>(uav->Desc);
            auto rhi = viewCache->GetOrCreateUAV(parentRHI, info);
            uav->RHIUnorderedAccessView = rhi;
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
                        auto* Tex = static_cast<RenderGraphTexture*>(SRV->Parent);
                        const auto& Desc = SRV->Desc;

                        RenderGraphPass::RenderGraphTextureIntent Intent;
                        Intent.Texture = Tex;

                        Intent.SubresourceRange = RHISubresourceRange(
                            Desc.FirstMipSlice,
                            Desc.FirstArraySlice,
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
                        auto* Tex = static_cast<RenderGraphTexture*>(UAV->Parent);
                        const auto& Desc = UAV->Desc;

                        RenderGraphPass::RenderGraphTextureIntent Intent;
                        Intent.Texture = Tex;

                        Intent.SubresourceRange = RHISubresourceRange(
                            Desc.FirstMipSlice,
                            Desc.FirstArraySlice,
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
                    auto vDesc = SRV->Desc;
                    if (SRV)
                    {
                        auto* Buf = static_cast<RenderGraphBuffer*>(SRV->Parent);

                        RenderGraphPass::RenderGraphBufferIntent Intent;
                        Intent.Buffer = Buf;

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
					auto uavDesc = UAV->Desc;
                    if (UAV)
                    {
                        auto* Buf = static_cast<RenderGraphBuffer*>(UAV->Parent);

                        RenderGraphPass::RenderGraphBufferIntent Intent;
                        Intent.Buffer = Buf;

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
        // =========================
    // Texture
    // =========================
        for (auto& [Key, Access] : FinalTextureStates)
        {
            auto* tex = Key.Texture;
            if (!tex || !tex->IsExternal)
                continue;

            RHI::RHITexture* rhiTex = tex->GetRHITexture();
            if (!rhiTex)
                continue;

            auto it = ExternalTextures.find(rhiTex);
            if (it == ExternalTextures.end())
                continue;

            ExternalTextureEntry& entry = it->second;
            if (!entry.Tracker)
                continue;

            // ✅ 写回 subresource
            entry.Tracker->UpdateSubresourceAccess(Key.Range, Access);
        }

        // =========================
        // Buffer
        // =========================
        for (auto& [Key, Access] : FinalBufferStates)
        {
            auto* buf = Key.Buffer;
            if (!buf || !buf->IsExternal)
                continue;

            RHI::RHIBuffer* rhiBuf = buf->GetRHIBuffer();
            if (!rhiBuf)
                continue;

            auto it = ExternalBuffers.find(rhiBuf);
            if (it == ExternalBuffers.end())
                continue;

            ExternalBufferEntry& entry = it->second;
            if (!entry.Tracker)
                continue;

            // =========================
            // 👉 如果你支持 range（推荐）
            // =========================

            // ❗ fallback：整 buffer
            entry.Tracker->UpdateAccess(Access);

        }
    }


} // namespace RenderCore
