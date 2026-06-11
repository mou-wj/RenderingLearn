// Scene.h
#pragma once
#include "SceneInterface.h"
#include "RenderResource.h"
#include "Flags.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <mutex>
namespace Engine
{
    class PrimitiveComponent;
    class PrimitiveSceneProxy;

    class LightComponent;
    class LightSceneProxy;

    class SceneView;
}
namespace Renderer {
    //====================================================
    // Primitive Scene Info
    //====================================================

    class PrimitiveSceneInfo
    {
    public:

        explicit PrimitiveSceneInfo(
            std::unique_ptr<
            Engine::PrimitiveSceneProxy>
            InProxy);

        Engine::PrimitiveSceneProxy*
            GetProxy() const
        {
            return Proxy.get();
        }

    public:

        bool bVisible = true;
        bool bCastShadow = true;
        bool bDynamic = true;

        uint32_t VisibilityFrame = 0;

    private:

        std::unique_ptr<
            Engine::PrimitiveSceneProxy>
            Proxy;
    };

    //====================================================
    // Light Scene Info
    //====================================================

    class LightSceneInfo
    {
    public:

        explicit LightSceneInfo(
            std::unique_ptr<
            Engine::LightSceneProxy>
            InProxy);

        Engine::LightSceneProxy*
            GetProxy() const
        {
            return Proxy.get();
        }

    public:

        bool bVisible = true;

    private:

        std::unique_ptr<
            Engine::LightSceneProxy>
            Proxy;
    };

    //====================================================
    // Scene Commands
    //====================================================

    enum class ESceneCommandType
    {
        AddPrimitive,
        RemovePrimitive,

        AddLight,
        RemoveLight
    };

    struct SceneCommand
    {
        ESceneCommandType Type;

        union
        {
            Engine::PrimitiveComponent*
                PrimitiveComponent;

            Engine::LightComponent*
                LightComponent;
        };

        
        std::unique_ptr<Engine::PrimitiveSceneProxy>
            PrimitiveProxy;

        
        std::unique_ptr<Engine::LightSceneProxy>
            LightProxy;
    };


    enum class ESceneGPUResourceDirty : uint32_t
    {
        None = 0,

        DirectionalLight = 1 << 0,
        PointLight = 1 << 1,
        SpotLight = 1 << 2,
        RectLight = 1 << 3,

        Primitive = 1 << 4,

        AllLight =
        DirectionalLight |
        PointLight |
        SpotLight |
        RectLight,

        All = 0xFFFFFFFF
    };
    ENUM_CLASS_FLAGS(ESceneGPUResourceDirty, ESceneGPUResourceDirtys);

    struct SceneGPULightResourceInfo {
        uint32_t PointLightCount = 0;
        uint32_t SpotLightCount = 0;
        uint32_t DirectionalLightCount = 0;
        uint32_t RectLightCount = 0;
        RenderCore::RenderBufferSP PointLightBuffer = nullptr;
        RenderCore::RenderBufferSP SpotLightBuffer = nullptr;
        RenderCore::RenderBufferSP DirectionalLightBuffer = nullptr;
        RenderCore::RenderBufferSP RectLightBuffer = nullptr;
    };
    struct SceneGPUResourceInfo {
        SceneGPULightResourceInfo LightResourceInfo;
        ESceneGPUResourceDirtys DirtyFlags = ESceneGPUResourceDirty::None;
    };


    /*
    ===============================================================================
        Scene (SceneInterface 的完全体工业级实现)
    ===============================================================================
    */
    class RENDERER_API Scene : public Engine::SceneInterface {
    public:
        Scene();
        ~Scene() override;

        // -------------------------------------------------------------------------
        // 游戏线程组件注册接口 (GameThread Only)
        // -------------------------------------------------------------------------
        void AddPrimitive(Engine::PrimitiveComponent* Component) override;
        void RemovePrimitive(Engine::PrimitiveComponent* Component) override;
        void AddLight(Engine::LightComponent* Component) override;
        void RemoveLight(Engine::LightComponent* Component) override;
        // -------------------------------------------------------------------------
        // 渲染线程专属查询接口 (RenderThread Only)
        // -------------------------------------------------------------------------
        void FlushPendingUpdates() override;
        void NotifyComponentChanged(Engine::SceneComponent* Component) override;
        
        
        void ForEachPrimitiveInView(const Engine::SceneView& View, std::function<void(Engine::PrimitiveSceneProxy*)> Visitor);

        void ForEachLight(std::function<void(Engine::LightSceneProxy*)> Visitor);

        const SceneGPUResourceInfo& GetGPUResourceInfo() const;
    private:
        void UpdateGPUResourceIfNeeded();

        //=========================================
        // Render Thread Storage
        //=========================================

        std::unordered_map<
            Engine::PrimitiveComponent*,
            std::unique_ptr<
            PrimitiveSceneInfo>>
            PrimitiveInfos;

        std::unordered_map<
            Engine::LightComponent*,
            std::unique_ptr<
            LightSceneInfo>>
            LightInfos;

        //=========================================
        // GT -> RT command queue
        //=========================================

        std::vector<
            SceneCommand>
            PendingCommands;

        std::mutex
            PendingCommandMutex;
        SceneGPUResourceInfo GPUResourceInfo;
    };
} // namespace Renderer