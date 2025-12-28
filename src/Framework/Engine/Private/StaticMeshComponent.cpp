// StaticMeshComponent.cpp
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "PrimitiveSceneProxy.h"
namespace Engine {
StaticMeshComponent::StaticMeshComponent()
    : PrimitiveComponent(), Mesh(nullptr), ForcedLOD(-1) {}

StaticMeshComponent::~StaticMeshComponent() {
}

PrimitiveSceneProxy* StaticMeshComponent::CreateSceneProxy() const {
    if (!Mesh) return nullptr;

    // Create a proxy for this component using the mesh's CreateSceneProxy
    PrimitiveSceneProxy* MeshProxy = Mesh->CreateSceneProxy();
    if (!MeshProxy) return nullptr;

    // Copy transform into proxy's LocalToWorld (if proxy exposes it). Our prototype
    // PrimitiveSceneProxy has LocalToWorld array; set it here if accessible.
    // We downcast for StaticPrimitiveSceneProxy / PrimitiveSceneProxy in this prototype.
    // This is prototype-level and assumes the proxy exposes LocalToWorld member.
    // In production use a well-defined interface for setting transform via render commands.

    return MeshProxy;
}

Core::BoxSphereBounds StaticMeshComponent::CalcBounds(const FTransform& LocalToWorld) const {
    // Simplified: if mesh exists, return its bounds converted by LocalToWorld.
    // For prototype, just return component Bounds or mesh bounds if available.
    if (Mesh) {
        // We don't have transform math here; return stored mesh bounds as-is wrapped in FBoxSphereBounds placeholder.
        // Caller should replace with proper transform application.
    }
    return Bounds;
}
} // namespace Engine