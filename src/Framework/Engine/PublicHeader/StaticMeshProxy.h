// StaticMeshProxy.h
// SceneProxy specialization for StaticMesh. RenderThread-read-only representation.
#pragma once

#include "PrimitiveSceneProxy.h"
#include "EngineExport.h"
#include <memory>
namespace Engine {

class StaticMeshComponent;
class ENGINE_API StaticMeshProxy : public PrimitiveSceneProxy {
public:
    // Construct from shared render data and component transform.
    StaticMeshProxy(const StaticMeshComponent* InComponent);
    ~StaticMeshProxy() override;

    void GetMeshBatches(const SceneView& View, MeshBatchList& OutBatches) const override;
    bool HasStaticGeometry() const override { return true; }
    bool IsDynamic() const override { return false; }

private:
    // Keep shared ownership of render data to ensure lifetime while proxy exists on RenderThread.
    const StaticMeshComponent* MeshComponent;
};
} // namespace Engine