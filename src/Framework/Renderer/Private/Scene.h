// Scene.h
#pragma once
#include "SceneInterface.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <mutex>

namespace Renderer {
    // 跨线程渲染命令类型
    enum class ESceneCommandType {
        Add,
        Remove
    };

    // 跨线程封装的命令结构
    struct SceneCommand {
        ESceneCommandType Type;
        int32_t ProxyId;
        Engine::PrimitiveSceneProxy * Proxy; // 仅在 Add 时有效
    };

    /*
    ===============================================================================
        Scene (SceneInterface 的完全体工业级实现)
    ===============================================================================
    */
    class Scene : public Engine::SceneInterface {
    public:
        Scene();
        ~Scene() override;

        // -------------------------------------------------------------------------
        // 游戏线程组件注册接口 (GameThread Only)
        // -------------------------------------------------------------------------
        void AddPrimitive(Engine::PrimitiveComponent* Component) override;
        void RemovePrimitive(Engine::PrimitiveComponent* Component) override;

        // -------------------------------------------------------------------------
        // 渲染线程专属查询接口 (RenderThread Only)
        // -------------------------------------------------------------------------
        Engine::PrimitiveSceneProxy* GetSceneProxyById(int32_t ProxyId) override;
        void ForEachProxyInView(const Engine::SceneView& View, std::function<void(Engine::PrimitiveSceneProxy*)> Visitor) override;

        // -------------------------------------------------------------------------
        // 渲染器全局状态查询 (Renderer Queries)
        // -------------------------------------------------------------------------
        Engine::FrameIndex GetCurrentFrameIndex() const override { return CurrentFrame; }
        double GetCurrentTimeSeconds() const override { return CurrentTimeSeconds; }
        Engine::EFeatureLevel GetFeatureLevel() const override { return FeatureLevel; }
        bool IsFeatureEnabled(const char* FeatureName) const override;

        // -------------------------------------------------------------------------
        // 视图与空间裁剪支撑
        // -------------------------------------------------------------------------
        int32_t GetCullingStructureHandle() const override { return CullingStructureHandle; }

        // -------------------------------------------------------------------------
        // 多线程生命周期与同步控制 (Lifecycle / Synchronization)
        // -------------------------------------------------------------------------
        void FlushPendingUpdates() override;
        void BeginFrameRender() override;
        void EndFrameRender() override;

        // -------------------------------------------------------------------------
        // 状态设置接口 (通常由 Engine 或 Application 层在游戏轮询中更新，传递给渲染)
        // -------------------------------------------------------------------------
        void UpdateSceneTime(double InTimeSeconds) { NewTimeSeconds = InTimeSeconds; }
        void EnableFeature(const std::string& FeatureName) { EnabledFeatures.insert(FeatureName); }

    private:
        // 1. 【RenderThread 独占】场景中所有活跃的代理映射表
        std::unordered_map<int32_t, Engine::PrimitiveSceneProxy*> PrimitiveProxies;

        // 2. 【GameThread 独占】跟踪组件到 ID 的映射，保证安全发出销毁请求
        std::unordered_map<Engine::PrimitiveComponent*, int32_t> ComponentToProxyId;

        // 3. 【线程安全安全缓冲区】拼命快照挂起队列与互斥锁
        std::vector<SceneCommand> PendingCommands;
        std::mutex CommandQueueMutex;

        // 4. 场景核心上下文状态
        int32_t NextProxyId;
        Engine::FrameIndex CurrentFrame;

        // 采用双缓冲时间戳，防止游戏线程写入时渲染线程正在读取导致数据撕裂（Tearing）
        double CurrentTimeSeconds;
        double NewTimeSeconds;

        Engine::EFeatureLevel FeatureLevel;
        std::unordered_set<std::string> EnabledFeatures;

        // 空间加速结构句柄 (如 Octree / BVH 句柄，初始化为无效值 -1)
        int32_t CullingStructureHandle = -1;
    };
} // namespace Renderer