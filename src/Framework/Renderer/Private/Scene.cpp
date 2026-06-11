#include "Scene.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "LightSceneProxy.h"
#include "SceneShaderParameters.h"
#include <cstring>
using namespace Engine;


namespace Renderer {


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
    // Scene
    //====================================================

    Scene::Scene()
    {
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
        Component->AddSceneListener(this);
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
        Component->RemoveSceneListener(this);
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
        Component->AddSceneListener(this);
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
        Component->RemoveSceneListener(this);
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
                    break;
                }

                case ESceneCommandType::
                RemovePrimitive:
                {
                    PrimitiveInfos.erase(
                        Command
                        .PrimitiveComponent);

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

            }
        }
    }

    void Scene::ForEachPrimitiveInView(
        const Engine::SceneView&,
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

    void Scene::UpdateGPUResourceIfNeeded()
    {
        auto Dirty = GPUResourceInfo.DirtyFlags;

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
                Data.Direction = Proxy->GetDirection();
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
            LightRes.DirectionalLightBuffer->UploadData(GPUData.data(), GPUData.size());
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


    }

    

} // namespace Renderer