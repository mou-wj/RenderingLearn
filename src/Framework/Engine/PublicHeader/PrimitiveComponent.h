// PrimitiveComponent.h
// Lightweight GameThread-side primitive description for renderer integration.
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <vector>
#include <typeinfo>
#include "BoxSphereBounds.h"
#include "TypeIDCast.h"

namespace Engine {
	// Minimal placeholder types. Replace with engine math types in real project.
	struct FTransform { /* placeholder: implement transform math in engine */ };

	class SceneInterface;
	class PrimitiveSceneProxy;
	class UMaterialInterface;
	struct ViewInfo;

	enum class EMobility : uint8_t { Static = 0, Stationary = 1, Movable = 2 };

	// PrimitiveComponent: a lightweight, GameThread-only description of a renderable
	// primitive. It does NOT own GPU/RHI resources. To render, it creates a SceneProxy
	// snapshot which is enqueued to the RenderThread.
	class ENGINE_API PrimitiveComponent {
	public:
		PrimitiveComponent();
		virtual ~PrimitiveComponent();
		DECLARE_TYPE_ID_BASE_TYPE(PrimitiveComponent)
		// ------------------ Spatial (GameThread only) ------------------
	protected:
		FTransform LocalTransform;          // local transform
		FTransform WorldTransform;          // cached world transform (derived)
		Core::BoxSphereBounds Bounds;            // bounds used for frustum culling
		EMobility Mobility;                 // mobility hint

		// ------------------ Render state (GameThread only) ------------------
		bool bVisible;                      // visible / hidden

		// ------------------ Scene linkage (GameThread only bookkeeping) ------------------
		SceneInterface* OwnerScene;         // scene this component is registered with
		PrimitiveSceneProxy* PendingSceneProxy;      // bookkeeping pointer on GameThread (RenderThread owns proxy)

		// Debug
		const char* DebugName;

	public:
		// ------------------ Registration / lifecycle ------------------
		// Register/unregister with the scene. Must be called on GameThread.
		virtual void RegisterComponentWithScene(SceneInterface* Scene);
		virtual void UnregisterComponentFromScene();

		// ------------------ SceneProxy management (virtual, override in derived) ------------------
		// Create a SceneProxy snapshot on GameThread. Returned pointer will be passed
		// to the RenderThread which takes ownership and manages its lifetime.
		virtual PrimitiveSceneProxy* CreateSceneProxy() const;

		// Request the SceneProxy be destroyed on RenderThread. Should enqueue deletion.
		virtual void DestroySceneProxy(PrimitiveSceneProxy* Proxy) const;

		// Mark that render-related state changed (material/geometry/flags) and needs
		// update on the RenderThread.
		virtual void MarkRenderStateDirty();

		// Update transform from GameThread and notify RenderThread (lightweight update).
		virtual void UpdateTransform(const FTransform& NewLocalTransform);

		// Called when transform / bounds were updated on GameThread.
		virtual void OnTransformUpdated();

		// Calculate bounds (derived must implement to provide frustum culling bounds).
		virtual Core::BoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const = 0;

		// Quick GameThread-only visibility query.
		bool IsVisible() const;

		// Convenience: recompute WorldTransform/Bounds and call OnTransformUpdated().
		void UpdateWorldTransform();

		// Disallow copy (components are unique handles)
		PrimitiveComponent(const PrimitiveComponent&) = delete;
		PrimitiveComponent& operator=(const PrimitiveComponent&) = delete;
	};


	// Notes:
	// - PrimitiveComponent is GameThread-only. Do not touch its members from RenderThread.
	// - SceneProxy must be a RenderThread-only, immutable (or RenderThread-controlled) object
	//   that contains a snapshot of everything needed to render (transform, bounds, material IDs, flags).
	// - All cross-thread interactions must be performed via an explicit RenderCommand queue.
}