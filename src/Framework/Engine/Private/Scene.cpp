// Scene.cpp
#include "Scene.h"
#include "PrimitiveComponent.h"


#include <utility>
namespace Engine {
Scene::Scene()
    : NextProxyId(1), CurrentFrame(0), CurrentTimeSeconds(0.0), FeatureLevel(EFeatureLevel::High) {
}

Scene::~Scene() {
    // Delete any remaining proxies on destruction (assume called on RenderThread)
    for (auto &kv : Proxies) {
        delete kv.second;
    }
    Proxies.clear();
}

void Scene::AddPrimitive(PrimitiveComponent* Component) {
    if (!Component) return;
    // Create proxy snapshot on GameThread
    PrimitiveSceneProxy* Proxy = Component->CreateSceneProxy();
    if (!Proxy) return;

    // Assign id on GameThread and record mapping for removal.
    int32_t AssignedId = NextProxyId++;
    ComponentToProxyId[Component] = AssignedId;

}

void Scene::RemovePrimitive(PrimitiveComponent* Component) {
    if (!Component) return;
    auto it = ComponentToProxyId.find(Component);
    if (it == ComponentToProxyId.end()) return;
    int32_t Id = it->second;
    ComponentToProxyId.erase(it);

    // Enqueue deletion on RenderThread
   //RenderCommand::Enqueue([this, Id]() {
   //    auto itp = Proxies.find(Id);
   //    if (itp != Proxies.end()) {
   //        delete itp->second;
   //        Proxies.erase(itp);
   //    }
   //});
}

SceneProxy* Scene::GetSceneProxyById(int32_t ProxyId) {
    auto it = Proxies.find(ProxyId);
    if (it == Proxies.end()) return nullptr;
    return it->second;
}

void Scene::ForEachProxyInView(const View& /*view*/, std::function<void(SceneProxy*)> visitor) {
    for (auto &kv : Proxies) {
        visitor(kv.second);
    }
}

FrameIndex Scene::GetCurrentFrameIndex() const { return CurrentFrame; }
double Scene::GetCurrentTimeSeconds() const { return CurrentTimeSeconds; }
EFeatureLevel Scene::GetFeatureLevel() const { return FeatureLevel; }

bool Scene::IsFeatureEnabled(const char* FeatureName) const {
    if (!FeatureName) return false;
    return EnabledFeatures.find(std::string(FeatureName)) != EnabledFeatures.end();
}

void Scene::GetLightsForView(const View& /*view*/, std::vector<int32_t>& outLightIds) const {
    // Example stub: no lights.
    outLightIds.clear();
}

int32_t Scene::GetCullingStructureHandle() const { return 0; }

void Scene::FlushPendingUpdates() {
    //RenderCommand::FlushAndRunAll();
}

void Scene::BeginFrameRender() {
    ++CurrentFrame;
    // In a real engine we'd update CurrentTimeSeconds from a time source
}

void Scene::EndFrameRender() {
    // finalize per-frame tasks
}
} // namespace Engine