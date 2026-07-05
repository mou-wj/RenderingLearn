// PrimitiveComponent.cpp
// Lightweight implementation examples and RenderCommand stubs.

#include "PrimitiveComponent.h"
#include <functional>
#include <utility>
namespace Engine {
    // PrimitiveComponent implementation
    PrimitiveComponent::PrimitiveComponent()
        : LocalTransform(), WorldTransform(), Bounds(), Mobility(EMobility::Static),
        bVisible(true), PendingSceneProxy(nullptr), DebugName(nullptr) {
    }

    PrimitiveComponent::~PrimitiveComponent() {
        // Ensure scene proxy is destroyed on RenderThread if still present
        if (PendingSceneProxy) {
            DestroySceneProxy(PendingSceneProxy);
            PendingSceneProxy = nullptr;
        }
        // Note: do NOT call OwnerScene removal here — caller should explicitly Unregister
    }

    void PrimitiveComponent::RegisterComponentWithScene(SceneInterface* Scene) {
        OwnerScene = Scene;
        // Scene is expected to perform bookkeeping on GameThread. Optionally, create proxy now.
        PrimitiveSceneProxy* Proxy = CreateSceneProxy();
        if (Proxy) {
            PendingSceneProxy = Proxy; // bookkeeping on GameThread
            // Transfer ownership to RenderThread / Scene via a render command.
            //RenderCommand::Enqueue([Scene, Proxy]() {
            //    // In real engine: Scene->OnAddPrimitive_RenderThread(Proxy);
            //    (void)Scene; (void)Proxy; // placeholder
            //});
        }
    }

    void PrimitiveComponent::UnregisterComponentFromScene() {
        // Caller must ensure GameThread context.
        OwnerScene = nullptr;
        if (PendingSceneProxy) {
            PrimitiveSceneProxy* Proxy = PendingSceneProxy;
            PendingSceneProxy = nullptr;
            DestroySceneProxy(Proxy);
        }
    }

    PrimitiveSceneProxy* PrimitiveComponent::CreateSceneProxy() const {
        // Default: no proxy. Derived classes should allocate and return a heap object
        // which contains a read-only snapshot of the primitive's state.
        return nullptr;
    }

    void PrimitiveComponent::DestroySceneProxy(PrimitiveSceneProxy* Proxy) const {
        if (!Proxy) return;
        // Schedule deletion on the RenderThread.
        //RenderCommand::Enqueue([Proxy]() {
        //    delete Proxy;
        //});
    }

    void PrimitiveComponent::MarkRenderStateDirty() {
        // Example: ask the Scene/RenderThread to recreate or update the proxy.
        if (!OwnerScene) return;
        //RenderCommand::Enqueue([this]() {
        //    // In real engine: OwnerScene->MarkPrimitiveDirty_RenderThread(sceneProxyId);
        //    (void)this;
        //});
    }

    void PrimitiveComponent::UpdateTransform(const FTransform& NewLocalTransform) {
        // Update on GameThread
        LocalTransform = NewLocalTransform;
        UpdateWorldTransform();

        // Send a lightweight transform update to RenderThread if proxy exists.
        PrimitiveSceneProxy* Proxy = PendingSceneProxy;
        if (Proxy) {
            // Capture a snapshot of world transform for the RenderThread.
            FTransform WorldSnapshot = WorldTransform;
            //RenderCommand::Enqueue([Proxy, WorldSnapshot]() {
            //    // In real engine: Proxy->SetWorldTransform(WorldSnapshot);
            //    (void)Proxy; (void)WorldSnapshot;
            //});
        }
    }

    void PrimitiveComponent::OnTransformUpdated() {
        // Default: nothing. Derived classes may override to react to transform changes.
    }

    void PrimitiveComponent::UpdateWorldTransform() {
        // For this lightweight example assume LocalTransform == WorldTransform.
        // In a real engine, compose with parent transforms here.
        WorldTransform = LocalTransform;
        Bounds = CalcBounds(WorldTransform);
        OnTransformUpdated();
    }

}