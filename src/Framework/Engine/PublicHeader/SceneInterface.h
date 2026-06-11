// SceneInterface.h
// Interface-only scene abstraction for Renderer-side access.
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace Engine {
    // 前置声明：只声明渲染线程可见的代理或视图结构
    class PrimitiveComponent;
    class PrimitiveSceneProxy; // 对齐重构后的 Proxy 名称
    class LightComponent;
    class SceneComponent;
    class SceneView;          // 对齐渲染视口体系

    using FrameIndex = uint64_t;

    enum class EFeatureLevel : uint8_t {
        Low = 0,
        Medium = 1,
        High = 2
    };

    /*
    ===============================================================================
        SceneInterface
        纯虚基类：描述了渲染器（Renderer）和渲染线程从一个场景实例中提取数据的所有契约
    ===============================================================================
    */
    class ENGINE_API SceneInterface {
    public:
        virtual ~SceneInterface() = default;

        // -------------------------------------------------------------------------
        // 游戏线程组件注册接口 (实现类内部必须将其包装为命令投递到 Pending 队列)
        // -------------------------------------------------------------------------
        virtual void AddPrimitive(PrimitiveComponent* Component) = 0;
        virtual void RemovePrimitive(PrimitiveComponent* Component) = 0;
        virtual void AddLight(LightComponent* Component) = 0;
        virtual void RemoveLight(LightComponent* Component) = 0;
        virtual void FlushPendingUpdates() = 0;
        virtual void NotifyComponentChanged(SceneComponent* Component) = 0;
    };
} // namespace Engine