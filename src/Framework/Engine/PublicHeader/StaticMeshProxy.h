// StaticMeshProxy.h
// SceneProxy specialization for StaticMesh. RenderThread-read-only representation.
#pragma once

#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"
#include "EngineExport.h"
#include <memory>
namespace Engine {
class ENGINE_API StaticMeshProxy : public PrimitiveSceneProxy {
public:
    // Construct from shared render data and component transform.
    StaticMeshProxy(int32_t InPrimitiveId, std::shared_ptr<FStaticMeshRenderData> InRenderData, const float InLocalToWorld[16]);
    ~StaticMeshProxy() override;

    void GetMeshBatches(const SceneView& View, MeshBatchList& OutBatches) const override;
    bool HasStaticGeometry() const override { return true; }
    bool IsDynamic() const override { return false; }

private:
    // Keep shared ownership of render data to ensure lifetime while proxy exists on RenderThread.
    std::shared_ptr<FStaticMeshRenderData> RenderData;
};
} // namespace Engine