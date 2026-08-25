# Engine 模块架构说明

## 1. 模块定位

`Engine` 模块是整个项目中的核心运行时层，负责承载场景管理、视口管理、资源管理、摄像机和渲染相关的基础能力。它并不是单一类，而是由多个相互协作的抽象层和具体实现共同组成的模块体系。

从源码结构看，`src/Framework/Engine` 主要分为两部分：

- `PublicHeader/`：定义对外接口和核心类型
- `Private/`：实现模块注册、引擎实例和具体逻辑

对应的关键文件包括：

- `Engine.h`：定义抽象 `Engine` 基类
- `EngineModule.h`：定义模块入口，继承自 `Core::Module`
- `IEngine.h / IEngine.cpp`：具体引擎实例实现
- `SceneComponent.h` / `PrimitiveComponent.h` / `StaticMeshComponent.h`：场景和渲染对象层
- `Viewport.h` / `ViewportClient.h` / `SceneViewport.h`：视口与渲染窗口层
- `SceneInterface.h`：场景接口，连接游戏线程和渲染线程
- `Camera.h`：摄像机逻辑
- `AssetManager.h`：资产管理

---

## 2. 模块入口：`EngineModule`

`EngineModule` 是 `Engine` 模块的入口类，继承自 `Core::Module`：

```cpp
class ENGINE_API EngineModule : public Core::Module
```

它主要负责：

1. 模块注册：通过 `IMPLEMENT_SIMPLE_MODULE(EngineModule, "Engine")` 注册到 `Core::ModuleManager`
2. 启动时创建 `IEngine` 实例，并调用 `Init()`
3. 关闭时调用 `Shutdown()` 并释放相关资源

启动逻辑在 `EngineModule.cpp` 中：

```cpp
void EngineModule::StartupModule()
{
    EngineObj = std::make_unique<IEngine>();
    EngineObj->Init();
    GDistanceFieldMgr.Initialize();
    isLoaded = true;
}
```

这说明 `Engine` 模块的生命周期是由 `ModuleManager` 驱动的，而不是由业务层直接创建和销毁。

---

## 3. 抽象引擎接口：`Engine` 与 `IEngine`

### 3.1 基类 `Engine`

`Engine.h` 中定义了抽象接口：

```cpp
class ENGINE_API Engine
{
public:
    virtual void Init() = 0;
    virtual void Tick(float deltaTime) = 0;
    virtual void Shutdown() = 0;
};
```

它定义了引擎的统一生命周期接口：

- `Init()`：模块初始化
- `Tick(float deltaTime)`：每帧更新
- `Shutdown()`：退出清理

这是整个引擎模块的顶层抽象，可以视为“引擎运行时协议”。

### 3.2 派生实现 `IEngine`

`IEngine` 继承自 `Engine`：

```cpp
class ENGINE_API IEngine : public Engine
```

`IEngine::Tick()` 是当前项目中引擎帧更新的实际入口，`Loop::Run()` 中直接调用：

```cpp
Engine::GetEngineInstance()->Tick(deltaTime);
```

因此，`IEngine` 是当前实现版本的具体引擎对象，承担了运行时的帧调度和逻辑更新入口。

---

## 4. 引擎内部的核心派生关系

### 4.1 场景对象体系

整个场景系统的核心继承链如下：

```text
SceneComponent
    └── Camera

SceneComponent
    └── PrimitiveComponent
            └── StaticMeshComponent
```

#### `SceneComponent`

`SceneComponent` 是场景节点的基础类型，提供：

- 组件挂接关系：`AttachTo()` / `Detach()`
- 局部/世界变换：位置、旋转、缩放
- 场景归属：`SetSceneOwner()` / `GetSceneOwner()`
- 变换更新：`OnTransformChanged()`、`MarkTransformDirty()`

它是最底层的场景对象基类，用于表达节点树结构。

#### `Camera`

`Camera` 继承自 `SceneComponent`，属于视角与投影控制类。它负责：

- 视点位置、目标、上向量
- 透视/正交投影设置
- view/projection/view-projection matrix 计算
- WVP 矩阵和坐标变换

这意味着 `Camera` 本质上是一种“带有投影和观察逻辑的场景组件”。

#### `PrimitiveComponent`

`PrimitiveComponent` 也继承自 `SceneComponent`，用于表示可渲染对象的通用描述：

- 自身有 `LocalTransform` / `WorldTransform`
- 具备 `Bounds` 与可见性标志
- 承担场景注册和 SceneProxy 创建流程
- 提供 `CreateSceneProxy()`、`UpdateTransform()` 等接口

它是渲染对象的游戏线程描述层，不直接持有 GPU 资源；真正的渲染数据由 `PrimitiveSceneProxy` 承担。

#### `StaticMeshComponent`

`StaticMeshComponent` 继承自 `PrimitiveComponent`，是可渲染静态网格的具体实现：

- 持有 `StaticMesh* Mesh`
- 支持材质覆盖：`SetMaterial()`
- 支持 LOD 控制：`SetForcedLOD()`
- 实现 `CreateSceneProxy()` 与 `GetBounds()`

这体现了引擎中“通用渲染组件 -> 特定渲染组件”的继承扩展方式。

---

### 4.2 SceneProxy 渲染快照体系

与 `PrimitiveComponent` 对应，项目中还存在一条 `SceneProxy` 的派生体系：

```text
PrimitiveSceneProxy
    └── StaticMeshProxy
```

#### `PrimitiveSceneProxy`

`PrimitiveSceneProxy` 是渲染线程侧的快照对象，负责存储渲染所需的结构化数据：

- `PrimitiveId`
- `ProxyBounds`
- `LocalToWorld` / `WorldToLocal`
- `bVisible` / `bCastShadow` / `bOpaque`
- 需要渲染时的统一参数等

它提供了 `IsVisible()`、`CastsShadow()`、`IsOpaque()` 等接口，说明它是渲染线程的抽象视图。

#### `StaticMeshProxy`

`StaticMeshProxy` 继承自 `PrimitiveSceneProxy`，用于表示静态网格的渲染快照：

```cpp
class ENGINE_API StaticMeshProxy : public PrimitiveSceneProxy
```

它重写了：

- `HasStaticGeometry()`
- `IsDynamic()`

并且持有 `StaticMeshComponent*` 作为对应资源关联。

这套设计体现了典型的 GameThread / RenderThread 分层：

- `PrimitiveComponent`：游戏线程中的组件描述
- `PrimitiveSceneProxy`：渲染线程中的一份快照数据

两个层次之间通过显式的代理对象来解耦。

---

### 4.3 视口系统

视口层的继承关系如下：

```text
RenderTarget
    └── Viewport
            └── SceneViewport

ViewportClient : public SlateCore::EventHandler
```

#### `RenderTarget`

`RenderTarget` 定义了渲染目标接口：

```cpp
virtual RenderCore::RenderTexture* GetRenderTarget() = 0;
```

它是所有视口/渲染目标的基础契约。

#### `Viewport`

`Viewport` 继承自 `RenderTarget`，定义统一视口接口：

- `Draw()`
- `GetWidth()`
- `GetHeight()`

它的职责是封装窗口中的渲染视口逻辑。

#### `SceneViewport`

`SceneViewport` 继承自：

```cpp
class SceneViewport final : public Viewport, public SlateCore::SlateViewport
```

这说明它同时具备：

- 引擎视口能力（`Viewport`）
- Slate UI 窗口能力（`SlateCore::SlateViewport`）

它可以直接处理窗口尺寸、输入事件和渲染目标绑定，是整个引擎渲染输出的核心窗口对象。

#### `ViewportClient`

`ViewportClient` 继承自 `SlateCore::EventHandler`，表示它是一个事件处理器：

```cpp
class ENGINE_API ViewportClient : public SlateCore::EventHandler
```

它负责：

- `Draw(Viewport* InViewport)`
- `OnViewportResized()`
- `Tick(float DeltaTime)`

它与 `SceneViewport` 之间是典型的“客户端/视口控制器”关系。

---

## 5. 场景交互接口：`SceneInterface`

`SceneInterface` 不是具体类，而是一个抽象接口，用于连接游戏线程与渲染线程之间的场景数据流：

```cpp
class ENGINE_API SceneInterface {
public:
    virtual void AddPrimitive(PrimitiveComponent* Component) = 0;
    virtual void RemovePrimitive(PrimitiveComponent* Component) = 0;
    virtual void AddLight(LightComponent* Component) = 0;
    virtual void RemoveLight(LightComponent* Component) = 0;
    virtual void FlushPendingUpdates() = 0;
    virtual void NotifyComponentChanged(SceneComponent* Component) = 0;
};
```

它的价值在于：

- 游戏线程负责组件更新
- 渲染线程负责读取相应的 `SceneProxy`
- 通过 `SceneInterface` 统一触发场景变更、更新和同步

从设计上看，它起到了“场景管理协议”的作用，属于引擎模块的中间枢纽。

---

## 6. `SceneObject`：场景对象封装层

`SceneObject` 不是派生自 `SceneComponent`，而是对 `SceneComponent` 的外部包装：

```cpp
class ENGINE_API SceneObject
```

其内部持有：

```cpp
SceneComponent* RootComponent = nullptr;
```

它的职责是：

- 作为一个逻辑对象，将一个根组件挂接到对象上
- 通过 `GetRootComponent<T>()` 提供类型安全访问
- 使得场景对象具有“组件树”组织方式

它是一个更高层的对象封装层，通常用于对组件树的统一管理。

---

## 7. 资源管理：`AssetManager` / `AssetRegistry`

`Engine` 模块还包含一套资源管理系统：

- `AssetRegistry`：记录资产信息和索引
- `AssetManager`：负责同步/异步加载资产

它们为场景和渲染资源的加载提供基础能力，属于引擎模块的“内容管理”部分。

典型设计是：

```cpp
class ENGINE_API AssetManager
{
public:
    template<typename AssetType>
    std::shared_ptr<AssetType> LoadSync(const std::string& Path);
};
```

这说明引擎不仅负责场景与渲染，还会统一管理资源加载与缓存，方便后续模型、材质、纹理等内容进入运行时系统。

---

## 8. 引擎模块的架构分层

综合源码分析，当前 `Engine` 模块可以抽象成以下分层：

```text
[Engine 模块入口]
    EngineModule
        ↓
    Engine / IEngine
        ↓
    场景与对象层
        SceneObject
        SceneComponent
        Camera
        PrimitiveComponent
        StaticMeshComponent
        ↓
    渲染快照层
        PrimitiveSceneProxy
        StaticMeshProxy
        ↓
    视口与窗口层
        RenderTarget
        Viewport
        SceneViewport
        ViewportClient
        ↓
    资源管理层
        AssetRegistry
        AssetManager
        ↓
    场景接口层
        SceneInterface
```

可以看到，当前引擎模块的核心思路并不是“一个大类塞满所有功能”，而是采用了典型的分层设计：

- 组件层：负责逻辑对象和场景节点
- 代理层：负责渲染线程访问
- 视口层：负责窗口和渲染输出
- 资源层：负责内容加载与登记
- 模块入口层：负责生命周期管理

---

## 9. 总结

`Engine` 模块的架构特点可以概括为：

1. 以 `Core::Module` 为入口，由 `EngineModule` 注册并管理生命周期。
2. 由 `Engine` 抽象接口统一定义 `Init / Tick / Shutdown`。 
3. `IEngine` 作为具体实现，负责引擎主循环的实际运行。
4. 场景系统采用从 `SceneComponent` 到 `PrimitiveComponent` 再到 `StaticMeshComponent` 的递进继承设计。
5. 渲染侧采用 `PrimitiveSceneProxy` 到 `StaticMeshProxy` 的快照代理模式来解耦游戏线程和渲染线程。
6. 视口系统采用 `Viewport` / `ViewportClient` / `SceneViewport` 的桥接结构，实现渲染输出和输入事件控制。
7. 资源管理和场景接口为引擎的各类功能提供统一扩展入口。

因此，当前 `Engine` 模块不是简单的“单一逻辑类”，而是一个以分层对象体系和模块化生命周期为核心的中间层架构。