#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Scene.h"
#include "SceneView.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "LightSceneProxy.h"
#include "SceneShaderParameters.h"
#include "FrustumCullPass.h"
#include "RenderResource.h"
#include "RHIPipelineStateCache.h"
#include "ShaderParameter.h"
#include "Shader.h"
#include "RHIApi.h"
#include "StaticMeshProxy.h"
#include "LocalVertexFactory.h"

#include <cstring>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Engine;


namespace Renderer {

    namespace
    {
        inline bool IntersectsAABB(const Core::AABB& A, const Core::AABB& B)
        {
            if (A.IsEmpty() || B.IsEmpty())
            {
                return false;
            }

            return !(A.Max.x < B.Min.x || A.Min.x > B.Max.x ||
                A.Max.y < B.Min.y || A.Min.y > B.Max.y ||
                A.Max.z < B.Min.z || A.Min.z > B.Max.z);
        }

        RHI::RHITextureDesc BuildDistanceFieldTextureArrayDesc(uint32_t arraySize, uint32_t textureSize, const char* debugName)
        {
            RHI::RHITextureDesc Desc;
            Desc.Width = textureSize;
            Desc.Height = textureSize;
            Desc.Depth = 1;
            Desc.MipLevels = 1;
            Desc.ArraySize = arraySize;
            Desc.Type = RHI::ERHITextureType::Texture2DArray;
            Desc.Format = RHI::ERHIFormat::R32_Float;
            Desc.SampleCount = 1;
            Desc.InitialQueueType = RHI::EQueueType::Compute;
            Desc.Usage =
                RHI::ERHITextureCreateFlag::ShaderResource |
                RHI::ERHITextureCreateFlag::TransferDest;
            Desc.DebugName = debugName;
            return Desc;
        }

        RHI::RHIBufferDesc BuildStructuredBufferDesc(uint64_t sizeInBytes, uint32_t stride, const char* debugName)
        {
            RHI::RHIBufferDesc Desc;
            Desc.Size = std::max<uint64_t>(1ull, sizeInBytes);
            Desc.Stride = stride;
            Desc.InitialQueueType = RHI::EQueueType::Compute;
            Desc.Usage =
                RHI::ERHIBufferUsageFlag::Structured |
                RHI::ERHIBufferUsageFlag::ShaderResource |
                RHI::ERHIBufferUsageFlag::TransferDst;
            Desc.DebugName = debugName;
            return Desc;
        }

        std::vector<float> BuildDistanceFieldSlice(
            const Core::AABB& blockBounds,
            uint32_t textureSize,
            const std::vector<Core::AABB>& sourceBounds)
        {
            std::vector<float> distances(textureSize * textureSize, 1000.0f);
            if (sourceBounds.empty() || blockBounds.IsEmpty())
            {
                return distances;
            }

            const float sizeX = blockBounds.Max.x - blockBounds.Min.x;
            const float sizeY = blockBounds.Max.y - blockBounds.Min.y;
            const float z = (blockBounds.Min.z + blockBounds.Max.z) * 0.5f;
            const float stepX = textureSize > 1 ? sizeX / static_cast<float>(textureSize - 1) : 0.0f;
            const float stepY = textureSize > 1 ? sizeY / static_cast<float>(textureSize - 1) : 0.0f;

            for (uint32_t y = 0; y < textureSize; ++y)
            {
                const float worldY = blockBounds.Min.y + static_cast<float>(y) * stepY;
                for (uint32_t x = 0; x < textureSize; ++x)
                {
                    const float worldX = blockBounds.Min.x + static_cast<float>(x) * stepX;

                    float minDistance = std::numeric_limits<float>::max();
                    for (const Core::AABB& source : sourceBounds)
                    {
                        const float dx = std::max(std::max(source.Min.x - worldX, 0.0f), worldX - source.Max.x);
                        const float dy = std::max(std::max(source.Min.y - worldY, 0.0f), worldY - source.Max.y);
                        const float dz = std::max(std::max(source.Min.z - z, 0.0f), z - source.Max.z);
                        const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                        if (distance < minDistance)
                        {
                            minDistance = distance;
                        }
                    }

                    distances[y * textureSize + x] = minDistance;
                }
            }

            return distances;
        }
    }


    //====================================================
    // PrimitiveSceneInfo
    //====================================================

    PrimitiveSceneInfo::
        PrimitiveSceneInfo(
            std::unique_ptr<
            Engine::PrimitiveSceneProxy>
            InProxy)
        :
        Proxy(std::move(InProxy))
    {
    }

    //====================================================
    // LightSceneInfo
    //====================================================

    LightSceneInfo::
        LightSceneInfo(
            std::unique_ptr<
            Engine::LightSceneProxy>
            InProxy)
        :
        Proxy(std::move(InProxy))
    {
    }

    //====================================================
    // SceneAccelerationStructure

    void SceneAccelerationStructure::Clear()
    {
        Nodes.clear();
        Primitives.clear();
    }

    void SceneAccelerationStructure::Build(const std::unordered_map<Engine::PrimitiveComponent*, std::unique_ptr<PrimitiveSceneInfo>>& PrimitiveInfos)
    {
        Primitives.clear();
        Nodes.clear();

        Primitives.reserve(PrimitiveInfos.size());
        for (auto& Pair : PrimitiveInfos)
        {
            if (!Pair.second) continue;
            if (!Pair.second->bVisible) continue;
            auto* Proxy = Pair.second->GetProxy();
            if (Proxy)
            {
                Primitives.push_back(Proxy);
            }
        }

        if (Primitives.empty())
        {
            return;
        }

        Nodes.reserve(Primitives.size() * 2);
        BuildNode(0, static_cast<int>(Primitives.size()));
    }

    int SceneAccelerationStructure::Partition(int Start, int Count, int Axis)
    {
        auto Compare = [Axis](Engine::PrimitiveSceneProxy* A, Engine::PrimitiveSceneProxy* B) {
            const Core::AABB& ABox = A->GetBounds().Box;
            const Core::AABB& BBox = B->GetBounds().Box;
            float ACenter = (Axis == 0 ? (ABox.Min.x + ABox.Max.x) : (Axis == 1 ? (ABox.Min.y + ABox.Max.y) : (ABox.Min.z + ABox.Max.z)));
            float BCenter = (Axis == 0 ? (BBox.Min.x + BBox.Max.x) : (Axis == 1 ? (BBox.Min.y + BBox.Max.y) : (BBox.Min.z + BBox.Max.z)));
            return ACenter < BCenter;
        };

        int Mid = Start + Count / 2;
        std::nth_element(Primitives.begin() + Start, Primitives.begin() + Mid, Primitives.begin() + Start + Count, Compare);
        return Mid;
    }

    int SceneAccelerationStructure::BuildNode(int Start, int Count)
    {
        int NodeIndex = static_cast<int>(Nodes.size());
        Nodes.emplace_back();
        Node& Current = Nodes.back();
        Current.Start = Start;
        Current.Count = Count;
        Current.LeftChild = -1;
        Current.RightChild = -1;
        Current.bLeaf = false;
        Current.Bounds.SetEmpty();

        for (int Index = Start; Index < Start + Count; ++Index)
        {
            Current.Bounds.Merge(Primitives[Index]->GetBounds());
        }

        if (Count <= 1)
        {
            Current.bLeaf = true;
            return NodeIndex;
        }

        Core::Float3 Extent = Current.Bounds.Box.GetExtent();
        int Axis = 0;
        if (Extent.y > Extent.x) Axis = 1;
        if (Extent.z > ((Axis == 0) ? Extent.x : Extent.y)) Axis = 2;

        int Mid = Partition(Start, Count, Axis);
        if (Mid <= Start || Mid >= Start + Count)
        {
            Current.bLeaf = true;
            return NodeIndex;
        }

        Current.LeftChild = BuildNode(Start, Mid - Start);
        Current.RightChild = BuildNode(Mid, Start + Count - Mid);
        return NodeIndex;
    }

    void SceneAccelerationStructure::Query(const Engine::SceneView& View, std::vector<Engine::PrimitiveSceneProxy*>& OutPrimitives) const
    {
        if (Nodes.empty())
        {
            return;
        }

        auto QueryNode = [&](auto&& self, int NodeIndex) -> void {
            const Node& Current = Nodes[NodeIndex];
            if (!View.IsBoxVisible(Current.Bounds.Box))
            {
                return;
            }

            if (Current.bLeaf)
            {
                for (int Index = Current.Start; Index < Current.Start + Current.Count; ++Index)
                {
                    OutPrimitives.push_back(Primitives[Index]);
                }
                return;
            }

            if (Current.LeftChild != -1)
            {
                self(self, Current.LeftChild);
            }
            if (Current.RightChild != -1)
            {
                self(self, Current.RightChild);
            }
        };

        QueryNode(QueryNode, 0);
    }


    //====================================================
    // Scene
    //====================================================

    Scene::Scene()
    {
		ShadowMapAllocator.Initialize(
			ShadowAllocatorDesc{
				{ 4096, 4096 } // SpotShadowAtlas
			});

                EnsureDistanceFieldResources();
    }

    Scene::~Scene()
    {
    }

    void Scene::AddPrimitive(
        Engine::PrimitiveComponent*
        Component)
    {
        if (!Component)
        {
            return;
        }
        if (PrimitiveInfos.find(Component) != PrimitiveInfos.end()) {
            return;
        }
        Component->SetSceneOwner(this);
        auto Proxy =
            std::unique_ptr<Engine::PrimitiveSceneProxy>(Component
            ->CreateSceneProxy());

        if (!Proxy)
        {
            return;
        }

        SceneCommand Command;

        Command.Type =
            ESceneCommandType
            ::AddPrimitive;

        Command.PrimitiveComponent =
            Component;

        Command.PrimitiveProxy =
            std::move(Proxy);

        std::scoped_lock Lock(
            PendingCommandMutex);

        PendingCommands.push_back(
            std::move(Command));
    }

    void Scene::RemovePrimitive(
        Engine::PrimitiveComponent*
        Component)
    {
        if (PrimitiveInfos.find(Component) == PrimitiveInfos.end()) {
            return;
        }
        Component->SetSceneOwner(nullptr);
        SceneCommand Command;

        Command.Type =
            ESceneCommandType
            ::RemovePrimitive;

        Command.PrimitiveComponent =
            Component;

        std::scoped_lock Lock(
            PendingCommandMutex);

        PendingCommands.push_back(
            std::move(Command));
    }

    void Scene::AddLight(
        Engine::LightComponent*
        Component)
    {
        if (!Component)
        {
            return;
        }
        if (LightInfos.find(Component) != LightInfos.end()) {
            return;
        }
        Component->SetSceneOwner(this);
        auto Proxy =
            std::unique_ptr<Engine::LightSceneProxy>(Component
            ->CreateSceneProxy());

        if (!Proxy)
        {
            return;
        }

        SceneCommand Command;

        Command.Type =
            ESceneCommandType
            ::AddLight;

        Command.LightComponent =
            Component;

        Command.LightProxy =
            std::move(Proxy);

        std::scoped_lock Lock(
            PendingCommandMutex);

        PendingCommands.push_back(
            std::move(Command));
    }

    void Scene::RemoveLight(
        Engine::LightComponent*
        Component)
    {
        if (LightInfos.find(Component) == LightInfos.end()) {
            return;
        }
        Component->SetSceneOwner(nullptr);
        SceneCommand Command;

        Command.Type =
            ESceneCommandType
            ::RemoveLight;

        Command.LightComponent =
            Component;

        std::scoped_lock Lock(
            PendingCommandMutex);

        PendingCommands.push_back(
            std::move(Command));
    }

    void Scene::FlushPendingUpdates()
    {
        std::vector<
            SceneCommand>
            Commands;

        {
            std::scoped_lock Lock(
                PendingCommandMutex);

            Commands.swap(
                PendingCommands);
        }
        Engine::LightSceneProxy* LightProxy = nullptr;
        auto markLightDiry = [this](Engine::LightSceneProxy* LightProxy) {
            if (!LightProxy) return;
            auto lightType = LightProxy->GetLightType();
            switch (lightType)
            {
                case Engine::ELightType::Directional:
                    GPUResourceInfo.DirtyFlags |= ESceneGPUResourceDirty::DirectionalLight;
                    break;
                case Engine::ELightType::Point:
                    GPUResourceInfo.DirtyFlags |= ESceneGPUResourceDirty::PointLight;
                    break;
                case Engine::ELightType::Spot:
                    GPUResourceInfo.DirtyFlags |= ESceneGPUResourceDirty::SpotLight;
                case Engine::ELightType::Sky:
                    GPUResourceInfo.DirtyFlags |= ESceneGPUResourceDirty::SkyLight;
                    break;
                default:
                    break;
            }


         };
        for (auto& Command
            : Commands)
        {
            switch (Command.Type)
            {
                case ESceneCommandType::
                AddPrimitive:
                {
                    PrimitiveInfos[
                        Command
                            .PrimitiveComponent]
                        =
                        std::make_unique<
                        PrimitiveSceneInfo>(
                            std::move(
                                Command
                                .PrimitiveProxy));
                    GPUResourceInfo.DirtyFlags |=
                        ESceneGPUResourceDirty
                        ::Primitive;
                    break;
                }

                case ESceneCommandType::
                RemovePrimitive:
                {
                    PrimitiveInfos.erase(
                        Command
                        .PrimitiveComponent);

                    GPUResourceInfo.DirtyFlags |=
                        ESceneGPUResourceDirty
                        ::Primitive;

                    break;
                }

                case ESceneCommandType::
                AddLight:
                {
                    markLightDiry(Command.LightProxy.get());
                    LightInfos[
                        Command
                            .LightComponent]
                        =
                        std::make_unique<
                        LightSceneInfo>(
                            std::move(
                                Command
                                .LightProxy));
                    
                    break;
                }

                case ESceneCommandType::
                RemoveLight:
                {
                    markLightDiry(Command.LightProxy.get());
                    LightInfos.erase(
                        Command
                        .LightComponent);
                    
                    break;
                }

                default:
                    break;
            }
        }

		//build GPU resource info
        UpdateGPUResourceIfNeeded();
        //update scene bounds
        SceneBounds.SetEmpty();
        for (auto& primitive : PrimitiveInfos) {
            SceneBounds.Merge(primitive.second->GetProxy()->GetBounds());
        }
        AccelerationStructure.Build(PrimitiveInfos);

    }

    void Scene::NotifyComponentChanged(SceneComponent* Component)
    {
        if (Component->IsA<Engine::LightComponent>())
        {
            auto* Light = static_cast<Engine::LightComponent*>(Component);
            switch (Light->GetLightType())
            {
            case ELightType::Directional:
                GPUResourceInfo.DirtyFlags |=
                    ESceneGPUResourceDirty
                    ::DirectionalLight;
                break;

            case ELightType::Point:
                GPUResourceInfo.DirtyFlags |=
                    ESceneGPUResourceDirty
                    ::PointLight;
                break;

            case ELightType::Spot:
                GPUResourceInfo.DirtyFlags |=
                    ESceneGPUResourceDirty
                    ::SpotLight;
                break;
            case ELightType::Sky:
                GPUResourceInfo.DirtyFlags |=
                    ESceneGPUResourceDirty
                    ::SkyLight;
                break;
            }
        }

        if (Component->IsA<Engine::PrimitiveComponent>())
        {
            GPUResourceInfo.DirtyFlags |=
                ESceneGPUResourceDirty
                ::Primitive;
        }
    }

    void Scene::ForEachPrimitive(
        std::function<void(
            Engine::PrimitiveSceneProxy*)>
        Visitor)
    {
        for (auto& Pair
            : PrimitiveInfos)
        {
            auto* Info =
                Pair.second.get();

            if (!Info->bVisible)
            {
                continue;
            }

            Visitor(
                Info->GetProxy());
        }
    }

    void Scene::ForEachLight(
        std::function<void(
            Engine::LightSceneProxy*)>
        Visitor)
    {
        for (auto& Pair
            : LightInfos)
        {
            auto* Info =
                Pair.second.get();

            if (!Info->bVisible)
            {
                continue;
            }

            Visitor(
                Info->GetProxy());
        }
    }

    const SceneGPUResourceInfo& Scene::GetGPUResourceInfo() const
    {
        return GPUResourceInfo;
    }

    ShadowMapAllocator& Scene::GetShadowMapAllocator()
    {
        return ShadowMapAllocator;
    }

    const Core::BoxSphereBounds& Scene::GetSceneBounds() const
    {
        return SceneBounds;
    }

    LightShadowInfo& Scene::GetLightShadowInfo(Engine::LightSceneProxy* Light)
    {
        return GPUResourceInfo.ShadowResourceInfo.LightShadowInfos[Light];
    }

    std::vector<Engine::PrimitiveSceneProxy*> Scene::GatherVisiblePrimitivesCPU(const Engine::SceneView& View) const
    {
        std::vector<Engine::PrimitiveSceneProxy*> Result;
        Result.reserve(PrimitiveInfos.size());

        if (AccelerationStructure.IsValid())
        {
            AccelerationStructure.Query(View, Result);
            return Result;
        }

        for (const auto& Pair : PrimitiveInfos)
        {
            const PrimitiveSceneInfo* Info = Pair.second.get();
            if (!Info || !Info->bVisible)
            {
                continue;
            }
            Engine::PrimitiveSceneProxy* Proxy = Info->GetProxy();
            if (!Proxy)
            {
                continue;
            }
            if (View.IsBoxVisible(Proxy->GetBounds().Box))
            {
                Result.push_back(Proxy);
            }
        }
        return Result;
    }

    std::vector<Engine::PrimitiveSceneProxy*> Scene::GatherVisiblePrimitivesGPU(const Engine::SceneView& View) const
    {
        std::vector<Engine::PrimitiveSceneProxy*> Result;
        const auto& PrimitiveRes = GPUResourceInfo.PrimitiveResourceInfo;
        Result.reserve(PrimitiveRes.PrimitiveCount);

        if (PrimitiveRes.PrimitiveCount == 0)
        {
            return Result;
        }

        if (!PrimitiveRes.PrimitiveBoundsBuffer ||
            !PrimitiveRes.VisibilityFlagsBuffer ||
            PrimitiveGPUProxyOrder.size() < PrimitiveRes.PrimitiveCount)
        {
            return GatherVisiblePrimitivesCPU(View);
        }

        FrustumCullPassInput PassInput;
        PassInput.ViewProjection = View.ViewProjectionMatrix;
        PassInput.PrimitiveCount = PrimitiveRes.PrimitiveCount;
        PassInput.PrimitiveBoundsBuffer = PrimitiveRes.PrimitiveBoundsBuffer.get();
        PassInput.VisibilityFlagsBuffer = PrimitiveRes.VisibilityFlagsBuffer.get();

        std::vector<uint32_t> visibilityFlags;
        if (!ExecuteFrustumCullPass(PassInput, visibilityFlags) ||
            visibilityFlags.size() < PrimitiveRes.PrimitiveCount)
        {
            return GatherVisiblePrimitivesCPU(View);
        }

        for (uint32_t index = 0; index < PrimitiveRes.PrimitiveCount; ++index)
        {
            if (visibilityFlags[index] == 0)
            {
                continue;
            }

            auto* proxy = PrimitiveGPUProxyOrder[index];
            if (proxy)
            {
                Result.push_back(proxy);
            }
        }

        return Result;
    }

    std::vector<Engine::PrimitiveSceneProxy*> Scene::GatherVisiblePrimitives(const Engine::SceneView& View) const
    {
        return GatherVisiblePrimitivesCPU(View);
    }

    void Scene::UpdatePrimitiveGPUResource()
    {
        auto& PrimitiveRes = GPUResourceInfo.PrimitiveResourceInfo;
        auto& LocalVFInfo = PrimitiveRes.localvertexfactoryinstanceInfo;

        PrimitiveGPUProxyOrder.clear();
        PrimitiveGPUProxyOrder.reserve(PrimitiveInfos.size());

        std::vector<AABBParameters> BoundsData;
        BoundsData.reserve(PrimitiveInfos.size());

        for (const auto& Pair : PrimitiveInfos)
        {
            const PrimitiveSceneInfo* Info = Pair.second.get();
            if (!Info || !Info->bVisible)
            {
                continue;
            }

            Engine::PrimitiveSceneProxy* Proxy = Info->GetProxy();
            if (!Proxy)
            {
                continue;
            }

            const Core::AABB& Bounds = Proxy->GetBounds().Box;
            AABBParameters Data;
            Data.Min = Bounds.Min;
            Data.Max = Bounds.Max;

            PrimitiveGPUProxyOrder.push_back(Proxy);
            BoundsData.emplace_back(Data);
        }

        PrimitiveRes.PrimitiveCount = static_cast<uint32_t>(BoundsData.size());



        RHI::RHIBufferDesc BoundsDesc;
        BoundsDesc.Size = std::max<uint64_t>(1ull, static_cast<uint64_t>(BoundsData.size() * sizeof(AABBParameters)));
        BoundsDesc.Stride = sizeof(AABBParameters);
        BoundsDesc.Usage =
            RHI::ERHIBufferUsageFlag::Structured |
            RHI::ERHIBufferUsageFlag::ShaderResource |
            RHI::ERHIBufferUsageFlag::TransferDst;
        BoundsDesc.InitialQueueType = RHI::EQueueType::Compute;
        PrimitiveRes.PrimitiveBoundsBuffer = std::make_shared<RenderCore::RenderBuffer>(BoundsDesc);
        PrimitiveRes.PrimitiveBoundsBuffer->InitRHIResource();
        if (!BoundsData.empty())
        {
            PrimitiveRes.PrimitiveBoundsBuffer->UploadData(
                BoundsData.data(),
                static_cast<uint32_t>(BoundsData.size() * sizeof(AABBParameters)));
        }

        RHI::RHIBufferDesc VisibilityDesc;
        VisibilityDesc.Size = std::max<uint64_t>(1ull, static_cast<uint64_t>(PrimitiveRes.PrimitiveCount * sizeof(uint32_t)));
        VisibilityDesc.Stride = sizeof(uint32_t);
        VisibilityDesc.Usage =
            RHI::ERHIBufferUsageFlag::Structured |
            RHI::ERHIBufferUsageFlag::UnorderedAccess |
            RHI::ERHIBufferUsageFlag::TransferSrc |
            RHI::ERHIBufferUsageFlag::TransferDst;
        VisibilityDesc.InitialQueueType = RHI::EQueueType::Compute;
        PrimitiveRes.VisibilityFlagsBuffer = std::make_shared<RenderCore::RenderBuffer>(VisibilityDesc);
        PrimitiveRes.VisibilityFlagsBuffer->InitRHIResource();

        std::vector<uint32_t> ZeroVisibility(PrimitiveRes.PrimitiveCount, 0u);
        const uint32_t UploadSize = PrimitiveRes.PrimitiveCount > 0 ?
            PrimitiveRes.PrimitiveCount * sizeof(uint32_t) :
            sizeof(uint32_t);
        PrimitiveRes.VisibilityFlagsBuffer->UploadData(
            ZeroVisibility.empty() ? static_cast<const void*>(&PrimitiveRes.PrimitiveCount) : static_cast<const void*>(ZeroVisibility.data()),
            UploadSize);

        std::vector<Engine::LocalVertexFactoryInstanceSubParameters> SubParameters;
        SubParameters.reserve(PrimitiveGPUProxyOrder.size());
        LocalVFInfo.StaticMeshProxyToInstanceId.clear();

        for (Engine::PrimitiveSceneProxy* Proxy : PrimitiveGPUProxyOrder)
        {
            if (!Proxy)
            {
                continue;
            }

            Engine::StaticMeshProxy* StaticMeshProxy = Proxy->Cast<Engine::StaticMeshProxy>();
            if (!StaticMeshProxy)
            {
                continue;
            }

            Engine::LocalVertexFactoryInstanceSubParameters Param;
            Param.LocalToWorld = StaticMeshProxy->GetLocalToWorld();

            const uint32_t InstanceId = static_cast<uint32_t>(SubParameters.size());
            SubParameters.push_back(Param);
            LocalVFInfo.StaticMeshProxyToInstanceId.emplace(StaticMeshProxy, InstanceId);
        }

        auto& InstanceManager = Engine::LocalVertexFactoryInstanceManager::Get();
        if (LocalVFInfo.InstanceBlock)
        {
            InstanceManager.RemoveInstanceBlock(LocalVFInfo.InstanceBlock);
            LocalVFInfo.InstanceBlock.reset();
        }

        if (!SubParameters.empty())
        {
            LocalVFInfo.InstanceBlock = InstanceManager.CreateInstanceBlock("ScenePrimitiveLocalVFInstances");
            LocalVFInfo.InstanceBlock->AddSubParameters(SubParameters);
        }

        InstanceManager.UpdateGPUResources();
    }

    void Scene::EnsureDistanceFieldResources()
    {
        auto& DFRes = GPUResourceInfo.DistanceFieldResourceInfo;

        if (!DFRes.StaticDistanceFieldTextureArray)
        {
            auto StaticTexDesc = BuildDistanceFieldTextureArrayDesc(
                SceneStaticDistanceFieldTextureCount,
                SceneStaticDistanceFieldTextureSize,
                "SceneStaticDistanceFieldTextureArray");
            DFRes.StaticDistanceFieldTextureArray = std::make_shared<RenderCore::RenderTexture>(StaticTexDesc);
            DFRes.StaticDistanceFieldTextureArray->InitRHIResource();
        }

        if (!DFRes.DynamicDistanceFieldTextureArray)
        {
            auto DynamicTexDesc = BuildDistanceFieldTextureArrayDesc(
                SceneDynamicDistanceFieldTextureCount,
                SceneDynamicDistanceFieldTextureSize,
                "SceneDynamicDistanceFieldTextureArray");
            DFRes.DynamicDistanceFieldTextureArray = std::make_shared<RenderCore::RenderTexture>(DynamicTexDesc);
            DFRes.DynamicDistanceFieldTextureArray->InitRHIResource();
        }

        if (!DFRes.StaticDistanceFieldIdBuffer)
        {
            auto Desc = BuildStructuredBufferDesc(
                static_cast<uint64_t>(SceneStaticDistanceFieldTextureCount) * sizeof(uint32_t),
                sizeof(uint32_t),
                "SceneStaticDistanceFieldIdBuffer");
            DFRes.StaticDistanceFieldIdBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            DFRes.StaticDistanceFieldIdBuffer->InitRHIResource();
        }

        if (!DFRes.DynamicDistanceFieldIdBuffer)
        {
            auto Desc = BuildStructuredBufferDesc(
                static_cast<uint64_t>(SceneDynamicDistanceFieldTextureCount) * sizeof(uint32_t),
                sizeof(uint32_t),
                "SceneDynamicDistanceFieldIdBuffer");
            DFRes.DynamicDistanceFieldIdBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            DFRes.DynamicDistanceFieldIdBuffer->InitRHIResource();
        }

        if (!DFRes.DynamicDistanceFieldInstanceInfoBuffer)
        {
            auto Desc = BuildStructuredBufferDesc(
                static_cast<uint64_t>(SceneDynamicDistanceFieldTextureCount) * sizeof(DynamicDistanceFieldBlockInstanceInfo),
                sizeof(DynamicDistanceFieldBlockInstanceInfo),
                "SceneDynamicDistanceFieldInstanceInfoBuffer");
            DFRes.DynamicDistanceFieldInstanceInfoBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            DFRes.DynamicDistanceFieldInstanceInfoBuffer->InitRHIResource();
        }
    }

    void Scene::PrecomputeStaticDistanceFieldTextures()
    {
        EnsureDistanceFieldResources();
        auto& DFRes = GPUResourceInfo.DistanceFieldResourceInfo;

        std::vector<uint32_t> blockToTextureId(SceneStaticDistanceFieldTextureCount, SceneDistanceFieldInvalidTextureId);
        uint32_t staticBlockCount = 0;

        if (!SceneBounds.Box.IsEmpty())
        {
            const float blockSize = SceneDistanceFieldBlockWorldSize;

            const int32_t minX = static_cast<int32_t>(std::floor(SceneBounds.Box.Min.x / blockSize));
            const int32_t minY = static_cast<int32_t>(std::floor(SceneBounds.Box.Min.y / blockSize));
            const int32_t minZ = static_cast<int32_t>(std::floor(SceneBounds.Box.Min.z / blockSize));

            const int32_t maxX = static_cast<int32_t>(std::floor(SceneBounds.Box.Max.x / blockSize));
            const int32_t maxY = static_cast<int32_t>(std::floor(SceneBounds.Box.Max.y / blockSize));
            const int32_t maxZ = static_cast<int32_t>(std::floor(SceneBounds.Box.Max.z / blockSize));

            for (int32_t z = minZ; z <= maxZ && staticBlockCount < SceneStaticDistanceFieldTextureCount; ++z)
            {
                for (int32_t y = minY; y <= maxY && staticBlockCount < SceneStaticDistanceFieldTextureCount; ++y)
                {
                    for (int32_t x = minX; x <= maxX && staticBlockCount < SceneStaticDistanceFieldTextureCount; ++x)
                    {
                        Core::AABB blockBounds;
                        blockBounds.Min = Core::Float3(
                            static_cast<float>(x) * blockSize,
                            static_cast<float>(y) * blockSize,
                            static_cast<float>(z) * blockSize);
                        blockBounds.Max = Core::Float3(
                            blockBounds.Min.x + blockSize,
                            blockBounds.Min.y + blockSize,
                            blockBounds.Min.z + blockSize);

                        std::vector<Core::AABB> intersectedStaticPrimitives;
                        for (const auto& pair : PrimitiveInfos)
                        {
                            const PrimitiveSceneInfo* info = pair.second.get();
                            if (!info || !info->bVisible)
                            {
                                continue;
                            }

                            Engine::PrimitiveSceneProxy* proxy = info->GetProxy();
                            if (!proxy || !proxy->HasStaticGeometry())
                            {
                                continue;
                            }

                            const Core::AABB primitiveBounds = proxy->GetBounds().Box;
                            if (IntersectsAABB(blockBounds, primitiveBounds))
                            {
                                intersectedStaticPrimitives.push_back(primitiveBounds);
                            }
                        }

                        if (intersectedStaticPrimitives.empty())
                        {
                            continue;
                        }

                        blockToTextureId[staticBlockCount] = staticBlockCount;
                        auto sliceData = BuildDistanceFieldSlice(
                            blockBounds,
                            SceneStaticDistanceFieldTextureSize,
                            intersectedStaticPrimitives);

                        DFRes.StaticDistanceFieldTextureArray->UploadData(
                            sliceData.data(),
                            0,
                            staticBlockCount);
                        ++staticBlockCount;
                    }
                }
            }
        }

        DFRes.StaticDistanceFieldBlockCount = staticBlockCount;
        DFRes.StaticDistanceFieldIdBuffer->UploadData(
            blockToTextureId.data(),
            static_cast<uint32_t>(blockToTextureId.size() * sizeof(uint32_t)));
    }

    void Scene::PrecomputeDynamicDistanceFieldTextures()
    {
        EnsureDistanceFieldResources();
        auto& DFRes = GPUResourceInfo.DistanceFieldResourceInfo;

        std::vector<uint32_t> blockToTextureId(SceneDynamicDistanceFieldTextureCount, SceneDistanceFieldInvalidTextureId);
        std::vector<DynamicDistanceFieldBlockInstanceInfo> instanceInfos(
            SceneDynamicDistanceFieldTextureCount,
            DynamicDistanceFieldBlockInstanceInfo{});

        uint32_t dynamicBlockCount = 0;
        for (const auto& pair : PrimitiveInfos)
        {
            if (dynamicBlockCount >= SceneDynamicDistanceFieldTextureCount)
            {
                break;
            }

            const PrimitiveSceneInfo* info = pair.second.get();
            if (!info || !info->bVisible)
            {
                continue;
            }

            Engine::PrimitiveSceneProxy* proxy = info->GetProxy();
            if (!proxy || !proxy->IsDynamic())
            {
                continue;
            }

            const Core::AABB primitiveBounds = proxy->GetBounds().Box;
            if (primitiveBounds.IsEmpty())
            {
                continue;
            }

            blockToTextureId[dynamicBlockCount] = dynamicBlockCount;

            DynamicDistanceFieldBlockInstanceInfo& instance = instanceInfos[dynamicBlockCount];
            instance.BoundsMin = primitiveBounds.Min;
            instance.BoundsMax = primitiveBounds.Max;
            instance.DistanceFieldTextureId = dynamicBlockCount;

            const float extentX = primitiveBounds.Max.x - primitiveBounds.Min.x;
            const float extentY = primitiveBounds.Max.y - primitiveBounds.Min.y;
            const float extentZ = primitiveBounds.Max.z - primitiveBounds.Min.z;
            instance.TextureCoordTransform = Core::Float3(
                extentX > 1e-4f ? 1.0f / extentX : 0.0f,
                extentY > 1e-4f ? 1.0f / extentY : 0.0f,
                extentZ > 1e-4f ? 1.0f / extentZ : 0.0f);

            std::vector<Core::AABB> sourceBounds;
            sourceBounds.push_back(primitiveBounds);
            auto sliceData = BuildDistanceFieldSlice(
                primitiveBounds,
                SceneDynamicDistanceFieldTextureSize,
                sourceBounds);

            DFRes.DynamicDistanceFieldTextureArray->UploadData(
                sliceData.data(),
                0,
                dynamicBlockCount);

            ++dynamicBlockCount;
        }

        DFRes.DynamicDistanceFieldBlockCount = dynamicBlockCount;
        DFRes.DynamicDistanceFieldIdBuffer->UploadData(
            blockToTextureId.data(),
            static_cast<uint32_t>(blockToTextureId.size() * sizeof(uint32_t)));
        DFRes.DynamicDistanceFieldInstanceInfoBuffer->UploadData(
            instanceInfos.data(),
            static_cast<uint32_t>(instanceInfos.size() * sizeof(DynamicDistanceFieldBlockInstanceInfo)));
    }


    void Scene::UpdateGPUResourceIfNeeded()
    {
        auto Dirty = GPUResourceInfo.DirtyFlags;
        uint32_t lightIndex = 0;
        if (Dirty ==
            ESceneGPUResourceDirty
            ::None)
        {
            return;
        }

        auto& LightRes =
            GPUResourceInfo
            .LightResourceInfo;

        //------------------------------------------------
        // Directional
        //------------------------------------------------

        if (EnumHasAnyFlags(
            Dirty,
            ESceneGPUResourceDirty
            ::DirectionalLight))
        {
            std::vector<
                DirectionalLightData>
                GPUData;

            for (auto& Pair :
                LightInfos)
            {
                auto* Proxy =
                    Pair.second
                    ->GetProxy();

                if (!Proxy)
                    continue;

                if (Proxy
                    ->GetLightType()
                    != ELightType
                    ::Directional)
                {
                    continue;
                }

                DirectionalLightData
                    Data;
                Data.Common.Color = Proxy->GetColor();
                Data.Common.Intensity = Proxy->GetIntensity();
                Data.Common.LightId = lightIndex;
                LightIndexs[Proxy] = lightIndex;
                lightIndex++;
                auto DirProxy = dynamic_cast<DirectionalLightSceneProxy*>(Proxy);
                Data.Direction = DirProxy ? DirProxy->GetDirection() : Core::Float3(0.0f, -1.0f, 0.0f);
                GPUData.emplace_back(
                    Data);
                
            }

            LightRes.DirectionalLightCount = static_cast<uint32_t>(GPUData.size());
            RHI::RHIBufferDesc Desc;
            Desc.Size = GPUData.size() * sizeof(DirectionalLightData);
            if (Desc.Size == 0) {
                Desc.Size = 1;
            }
            Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
            LightRes.DirectionalLightBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            LightRes.DirectionalLightBuffer->InitRHIResource();
            LightRes.DirectionalLightBuffer->UploadData(GPUData.data(), Desc.Size);
        }

        //------------------------------------------------
        // Point
        //------------------------------------------------

        if (EnumHasAnyFlags(
            Dirty,
            ESceneGPUResourceDirty
            ::PointLight))
        {
            std::vector<
                PointLightData>
                GPUData;

            for (auto& Pair :
                LightInfos)
            {
                auto* Proxy =
                    Pair.second
                    ->GetProxy();

                if (!Proxy)
                    continue;

                if (Proxy
                    ->GetLightType()
                    != ELightType
                    ::Point)
                {
                    continue;
                }

                PointLightData
                    Data;
                Data.Common.Color = Proxy->GetColor();
                Data.Common.Intensity = Proxy->GetIntensity();
                Data.Position = Proxy->GetPosition();
                Data.Common.LightId = lightIndex;
                LightIndexs[Proxy] = lightIndex;
                lightIndex++;
                auto PointLightProxy = dynamic_cast<PointLightSceneProxy*>(Proxy);
                Data.Radius = PointLightProxy->GetAttenuationRadius();

                GPUData.emplace_back(
                    Data);
            }

            LightRes.PointLightCount =
                static_cast<uint32_t>(
                    GPUData.size());
            RHI::RHIBufferDesc Desc;
            Desc.Size = GPUData.size() * sizeof(PointLightData);
            if (Desc.Size == 0) {
                Desc.Size = 1;
            }
            Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
            LightRes.PointLightBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            LightRes.PointLightBuffer->InitRHIResource();
            LightRes.PointLightBuffer->UploadData(GPUData.data(), Desc.Size);
        }

        //------------------------------------------------
        // Spot
        //------------------------------------------------

        if (EnumHasAnyFlags(
            Dirty,
            ESceneGPUResourceDirty
            ::SpotLight))
        {
            std::vector<
                SpotLightData>
                GPUData;

            for (auto& Pair :
                LightInfos)
            {
                auto* Proxy =
                    Pair.second
                    ->GetProxy();

                if (!Proxy)
                    continue;

                if (Proxy
                    ->GetLightType()
                    != ELightType
                    ::Spot)
                {
                    continue;
                }

                SpotLightData
                    Data;
                Data.Common.Color = Proxy->GetColor();
                Data.Common.Intensity = Proxy->GetIntensity();
                Data.Common.LightId = lightIndex;
                LightIndexs[Proxy] = lightIndex;
                lightIndex++;
                Data.Position = Proxy->GetPosition();
                auto SpotLightProxy = dynamic_cast<SpotLightSceneProxy*>(Proxy);
                Data.Radius = SpotLightProxy->GetAttenuationRadius();
                Data.Direction = SpotLightProxy->GetDirection();
                Data.InnerConeCos = SpotLightProxy->GetInnerConeAngle();
                Data.OuterConeCos = SpotLightProxy->GetOuterConeAngle();

                GPUData.emplace_back(
                    Data);
            }

            LightRes.SpotLightCount =
                static_cast<uint32_t>(
                    GPUData.size());
            RHI::RHIBufferDesc Desc;
            Desc.Size = GPUData.size() * sizeof(SpotLightData);
            if (Desc.Size == 0) {
                Desc.Size = 1;
            }
            Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
            LightRes.SpotLightBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            LightRes.SpotLightBuffer->InitRHIResource();
            LightRes.SpotLightBuffer->UploadData(GPUData.data(), Desc.Size);
        }

        if (EnumHasAnyFlags(
            Dirty,
            ESceneGPUResourceDirty
            ::Primitive))
        {
            UpdatePrimitiveGPUResource();
            PrecomputeStaticDistanceFieldTextures();
            PrecomputeDynamicDistanceFieldTextures();
        }
        LightRes.IBLSpecularTexture = nullptr;
        LightRes.IBLSpecularTexture = nullptr;
        if (EnumHasAnyFlags(
            Dirty,
            ESceneGPUResourceDirty
            ::SkyLight)) {
            for (auto& Pair :
                LightInfos)
            {
                auto* Proxy =
                    Pair.second
                    ->GetProxy();

                if (!Proxy)
                    continue;

                if (Proxy
                    ->GetLightType()
                    != ELightType
                    ::Sky)
                {
                    continue;
                }
                auto SkyLightProxy = dynamic_cast<SkyLightSceneProxy*>(Proxy);
                LightRes.IBLSpecularTexture = SkyLightProxy->GetSpecularPrefilter();
                LightRes.IBLDiffuseTexture = SkyLightProxy->GetDiffuseIrradiance();
            }

        }
        
    }

    

} // namespace Renderer