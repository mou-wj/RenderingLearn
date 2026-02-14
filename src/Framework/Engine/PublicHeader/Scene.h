// Scene.h
// Concrete example implementation of SceneInterface (keeps minimal internal state).
#pragma once

#include "SceneInterface.h"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
namespace Engine {
// Forward
class PrimitiveComponent;
class SceneProxy;

class Scene : public SceneInterface {
public:
    Scene();
    ~Scene();

    // Primitive management
    void AddPrimitive(PrimitiveComponent* Component) override;
    void RemovePrimitive(PrimitiveComponent* Component) override;

    SceneProxy* GetSceneProxyById(int32_t ProxyId) override;
    void ForEachProxyInView(const View& view, std::function<void(SceneProxy*)> visitor) override;
    void ForEachPrimitiveComponent(std::function<void(PrimitiveComponent*)> visitor) override;
    // Renderer queries
    FrameIndex GetCurrentFrameIndex() const override;
    double GetCurrentTimeSeconds() const override;
    EFeatureLevel GetFeatureLevel() const override;
    bool IsFeatureEnabled(const char* FeatureName) const override;

    void GetLightsForView(const View& view, std::vector<int32_t>& outLightIds) const override;
    int32_t GetCullingStructureHandle() const override;

    // Lifecycle
    void FlushPendingUpdates() override;
    void BeginFrameRender() override;
    void EndFrameRender() override;

private:
    // RenderThread owned map of ProxyId -> SceneProxy*
    std::unordered_map<int32_t, SceneProxy*> Proxies;

    // GameThread-visible mapping Component* -> ProxyId (for removal requests)
    std::unordered_map<PrimitiveComponent*, int32_t> ComponentToProxyId;

    int32_t NextProxyId;
    FrameIndex CurrentFrame;
    double CurrentTimeSeconds;
    EFeatureLevel FeatureLevel;
    std::unordered_set<std::string> EnabledFeatures;
};
} // namespace Engine