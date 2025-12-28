// PrimitiveSceneProxy.cpp
#include "PrimitiveSceneProxy.h"
#include "SceneView.h"

#include <cstring>
namespace Engine {
PrimitiveSceneProxy::PrimitiveSceneProxy(int32_t InPrimitiveId)
    : PrimitiveId(InPrimitiveId), VertexBufferId(-1), IndexBufferId(-1), bVisible(true),
      bCastShadow(true), bOpaque(true), RenderFlags(0) {
    // initialize LocalToWorld to identity
    std::memset(LocalToWorld, 0, sizeof(LocalToWorld));
    LocalToWorld[0] = LocalToWorld[5] = LocalToWorld[10] = LocalToWorld[15] = 1.0f;
    
}

PrimitiveSceneProxy::~PrimitiveSceneProxy() {}

int32_t PrimitiveSceneProxy::GetMaterialId(int32_t ElementIndex) const {
    if (ElementIndex < 0 || ElementIndex >= (int32_t)MaterialIds.size()) return -1;
    return MaterialIds[ElementIndex];
}

// StaticPrimitiveSceneProxy
StaticPrimitiveSceneProxy::StaticPrimitiveSceneProxy(int32_t InPrimitiveId)
    : PrimitiveSceneProxy(InPrimitiveId), BatchIndexStart(0), BatchIndexCount(0), BatchMaterialId(-1) {}

StaticPrimitiveSceneProxy::~StaticPrimitiveSceneProxy() {}

void StaticPrimitiveSceneProxy::SetGeometry(ResourceId VB, ResourceId IB, uint32_t IndexStart, uint32_t IndexCount, int32_t MaterialId) {
    VertexBufferId = VB;
    IndexBufferId = IB;
    BatchIndexStart = IndexStart;
    BatchIndexCount = IndexCount;
    BatchMaterialId = MaterialId;
    MaterialIds.clear();
    MaterialIds.push_back(MaterialId);
}

void StaticPrimitiveSceneProxy::GetMeshBatches(const SceneView& /*View*/, MeshBatchList& OutBatches) const {
    MeshBatch mb;
    mb.VertexBufferId = VertexBufferId;
    mb.IndexBufferId = IndexBufferId;
    mb.IndexStart = BatchIndexStart;
    mb.IndexCount = BatchIndexCount;
    mb.MaterialId = BatchMaterialId;
    mb.SortKey = 0;
    OutBatches.push_back(mb);
}
} // namespace Engine