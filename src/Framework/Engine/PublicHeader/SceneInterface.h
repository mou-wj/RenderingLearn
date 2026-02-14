// SceneInterface.h
// Interface-only scene abstraction for Renderer-side access.
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <functional>
#include <vector>
namespace Engine {
// Forward declarations
class PrimitiveComponent;
class SceneProxy;
struct View;
struct ViewFamily;

using FrameIndex = uint64_t;

enum class EFeatureLevel : uint8_t { Low = 0, Medium = 1, High = 2 };

// SceneInterface: pure-virtual interface describing what Renderer needs from a Scene.
class ENGINE_API SceneInterface {
public:
    virtual ~SceneInterface() = default;

    // Primitive management (GameThread calls should enqueue requests)
    virtual void AddPrimitive(PrimitiveComponent* Component) = 0;
    virtual void RemovePrimitive(PrimitiveComponent* Component) = 0;

    // Access proxies by id (RenderThread only)
    virtual SceneProxy* GetSceneProxyById(int32_t ProxyId) = 0;
    virtual void ForEachProxyInView(const View& view, std::function<void(SceneProxy*)> visitor) = 0;
    virtual void ForEachPrimitiveComponent(std::function<void(PrimitiveComponent*)> visitor) = 0;

    // Renderer queries
    virtual FrameIndex GetCurrentFrameIndex() const = 0;
    virtual double GetCurrentTimeSeconds() const = 0;
    virtual EFeatureLevel GetFeatureLevel() const = 0;
    virtual bool IsFeatureEnabled(const char* FeatureName) const = 0;

    // View / culling support
    virtual void GetLightsForView(const View& view, std::vector<int32_t>& outLightIds) const = 0;
    virtual int32_t GetCullingStructureHandle() const = 0;

    // Lifecycle / synchronization
    virtual void FlushPendingUpdates() = 0; // ensure queued commands run
    virtual void BeginFrameRender() = 0;    // RenderThread: prepare for frame
    virtual void EndFrameRender() = 0;      // RenderThread: finalize frame
};
} // namespace Engine