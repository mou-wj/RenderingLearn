// StaticMeshProxy.cpp
#include "StaticMeshProxy.h"
#include "SceneView.h"

#include <cstring>
namespace Engine {
StaticMeshProxy::StaticMeshProxy(int32_t InPrimitiveId, std::shared_ptr<FStaticMeshRenderData> InRenderData, const float InLocalToWorld[16])
    : PrimitiveSceneProxy(InPrimitiveId), RenderData(std::move(InRenderData)) {
    // copy local-to-world matrix
    if (InLocalToWorld) std::memcpy(LocalToWorld, InLocalToWorld, sizeof(LocalToWorld));
}

StaticMeshProxy::~StaticMeshProxy() {}

void StaticMeshProxy::GetMeshBatches(const SceneView& /*View*/, MeshBatchList& OutBatches) const {
    if (!RenderData) return;
    // For prototype: take LOD 0 and output batches for each section
    if (RenderData->GetLODCount() == 0) return;
    const LODResource& L0 = RenderData->GetLODResource(0);
    // For each section, emit MeshBatch with index range and material id
    for (const SectionInfo& S : L0.Sections) {
        MeshBatch mb;
        mb.VertexBufferId = -1; // no GPU upload in prototype
        mb.IndexBufferId = -1;
        mb.IndexStart = S.FirstIndex;
        mb.IndexCount = S.NumIndices;
        mb.MaterialId = S.MaterialIndex;
        mb.SortKey = 0;
        OutBatches.push_back(mb);
    }
}
} // namespace Engine