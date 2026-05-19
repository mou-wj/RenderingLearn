#include "Scene.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveComponent.h"
#include <cstring>
using namespace Engine;


namespace Renderer {

    Scene::Scene()
        : NextProxyId(1)
        , CurrentFrame(0)
        , CurrentTimeSeconds(0.0)
        , NewTimeSeconds(0.0)
        , FeatureLevel(EFeatureLevel::High) // 默认开启高质量特征级别
        , CullingStructureHandle(0)         // 假设 0 代表根空间裁剪加速节点
    {
    }

    Scene::~Scene() {
        // 彻底清空所有遗留的渲染代理，防止内存泄漏
        for (auto& Pair : PrimitiveProxies) {
            delete Pair.second;
        }
        PrimitiveProxies.clear();

        // 销毁未录制的命令中的代理
        for (auto& Cmd : PendingCommands) {
            if (Cmd.Type == ESceneCommandType::Add && Cmd.Proxy) {
                delete Cmd.Proxy;
            }
        }
    }

    /*
    ===============================================================================
        游戏线程接口 (GameThread)
    ===============================================================================
    */
    void Scene::AddPrimitive(PrimitiveComponent* Component) {
        if (!Component) return;

        int32_t ProxyId = NextProxyId++;
        ComponentToProxyId[Component] = ProxyId;

        // 在游戏线程上下文，一次性将当前组件的所有物理状态快照打包进 Proxy
        PrimitiveSceneProxy* NewProxy = Component->CreateSceneProxy();

        // 锁定时长极短：仅入队命令，随后立刻释放锁，绝不阻塞游戏逻辑
        {
            std::lock_guard<std::mutex> Lock(CommandQueueMutex);
            PendingCommands.push_back({ ESceneCommandType::Add, ProxyId, NewProxy });
        }
    }

    void Scene::RemovePrimitive(PrimitiveComponent* Component) {
        auto It = ComponentToProxyId.find(Component);
        if (It != ComponentToProxyId.end()) {
            int32_t ProxyId = It->second;
            ComponentToProxyId.erase(It);

            // 发送销毁命令，绝对不要在此处 delete 指针！
            {
                std::lock_guard<std::mutex> Lock(CommandQueueMutex);
                PendingCommands.push_back({ ESceneCommandType::Remove, ProxyId, nullptr });
            }
        }
    }

    /*
    ===============================================================================
        渲染线程查询与生命周期接口 (RenderThread Only)
    ===============================================================================
    */
    PrimitiveSceneProxy* Scene::GetSceneProxyById(int32_t ProxyId) {
        auto It = PrimitiveProxies.find(ProxyId);
        return (It != PrimitiveProxies.end()) ? It->second : nullptr;
    }

    void Scene::ForEachProxyInView(const SceneView& View, std::function<void(PrimitiveSceneProxy*)> Visitor) {
        // 现代图形引擎核心关卡：此处对接你的八叉树或 BVH 空间裁剪
        // 在这里，我们先使用全场景遍历作为最稳健的 fallback 实现：
        for (const auto& Pair : PrimitiveProxies) {
            PrimitiveSceneProxy* Proxy = Pair.second;

            // 工业级框架在此处会执行：if (View.Frustum.Intersect(Proxy->GetBounds()))
            // 裁剪通过后，将可见代理回调投递给渲染器
            Visitor(Proxy);
        }
    }

    bool Scene::IsFeatureEnabled(const char* FeatureName) const {
        if (!FeatureName) return false;
        return EnabledFeatures.find(FeatureName) != EnabledFeatures.end();
    }

    void Scene::FlushPendingUpdates() {
        // 1. 采用 swap 极其优雅地将暂存队列置换到本地，瞬间解锁，不卡死游戏线程
        std::vector<SceneCommand> CommandsToExecute;
        {
            std::lock_guard<std::mutex> Lock(CommandQueueMutex);
            CommandsToExecute.swap(PendingCommands);
        }

        // 2. 处于绝对安全的渲染上下文中，批量执行代理的增删
        for (const auto& Cmd : CommandsToExecute) {
            if (Cmd.Type == ESceneCommandType::Add) {

                PrimitiveProxies[Cmd.ProxyId] = Cmd.Proxy;
                // 工业级框架预留：在此处将 Proxy 的 AABB 塞入加速结构 (如 Octree->Insert(Cmd.Proxy))

            }
            else if (Cmd.Type == ESceneCommandType::Remove) {

                auto It = PrimitiveProxies.find(Cmd.ProxyId);
                if (It != PrimitiveProxies.end()) {
                    PrimitiveSceneProxy* ProxyToDelete = It->second;
                    PrimitiveProxies.erase(It);

                    // 工业级框架预留：在此处从加速结构移除 (如 Octree->Remove(ProxyToDelete))

                    // 安全释放只读镜像
                    delete ProxyToDelete;
                }
            }
        }
    }

    void Scene::BeginFrameRender() {
        // 1. 自增渲染帧率计数
        CurrentFrame++;

        // 2. 将安全缓冲区的游戏时间戳拉取到当前渲染帧，彻底杜绝多线程时间读写撕裂
        CurrentTimeSeconds = NewTimeSeconds;

        // 3. 工业级框架在此处会重构或打包当前帧的场景全局 Uniform Buffer (如 ViewFamily / SceneGlobals)
    }

    void Scene::EndFrameRender() {
        // 每一帧结束后的后置清理（如清理当前帧的临时分配器、统计渲染帧数据等）
    }

} // namespace Engine