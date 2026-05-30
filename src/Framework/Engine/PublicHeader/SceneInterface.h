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
    struct SceneView;          // 对齐渲染视口体系

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

        // -------------------------------------------------------------------------
        // 渲染线程专属查询接口 (RenderThread Only)
        // -------------------------------------------------------------------------
        // 依据分配的唯一 ID 查找对应的渲染代理
        virtual PrimitiveSceneProxy* GetSceneProxyById(int32_t ProxyId) = 0;

        // 核心裁剪：遍历当前视口(SceneView)内可见的所有几何体代理
        virtual void ForEachProxyInView(const SceneView& View, std::function<void(PrimitiveSceneProxy*)> Visitor) = 0;

        // -------------------------------------------------------------------------
        // 渲染器全局状态查询 (Renderer Queries)
        // -------------------------------------------------------------------------
        virtual FrameIndex GetCurrentFrameIndex() const = 0;
        virtual double GetCurrentTimeSeconds() const = 0;
        virtual EFeatureLevel GetFeatureLevel() const = 0;
        virtual bool IsFeatureEnabled(const char* FeatureName) const = 0;

        // -------------------------------------------------------------------------
        // 视图与空间裁剪支撑
        // -------------------------------------------------------------------------
        // 未来灯光也会有对应的 LightSceneProxy*，此处预留指针列表收集接口，彻底废除整型 ID
        // virtual void GetLightsForView(const SceneView& View, std::vector<class LightSceneProxy*>& OutLights) const = 0;
        virtual int32_t GetCullingStructureHandle() const = 0;

        // -------------------------------------------------------------------------
        // 多线程生命周期与同步控制 (Lifecycle / Synchronization)
        // -------------------------------------------------------------------------
        // 在两线程交割或帧渲染前被触发：安全地消耗挂起的 Add/Remove 命令队列，刷新场景树
        virtual void FlushPendingUpdates() = 0;

        // 渲染线程：一帧渲染开始前的准备工作（如更新场景全局常量缓冲、动态 BVH 树重构等）
        virtual void BeginFrameRender() = 0;

        // 渲染线程：一帧渲染结束后的清理与回收
        virtual void EndFrameRender() = 0;
    };
} // namespace Engine