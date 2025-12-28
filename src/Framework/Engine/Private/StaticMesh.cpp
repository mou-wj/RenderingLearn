// StaticMesh.cpp
#include "StaticMesh.h"
#include "PrimitiveSceneProxy.h"
namespace Engine {
// Default CreateSceneProxy: create a StaticPrimitiveSceneProxy if render data exists.
PrimitiveSceneProxy* StaticMesh::CreateSceneProxy() const {
    auto RD = GetRenderData();
    if (!RD || RD->GetLODCount() == 0) return nullptr;

    // We create a simple StaticPrimitiveSceneProxy and populate it with LOD0 data
    // For more advanced implementations, create a specialized StaticMeshProxy.
    StaticPrimitiveSceneProxy* Proxy = new StaticPrimitiveSceneProxy(/*primitive id*/ 0);

    // If LOD0 has data, set geometry ids to placeholder values (-1) since
    // resource upload to GPU is outside scope of this prototype.
    // In a real engine, you'd create GPU buffers and map to ResourceId.
    // Here we leave ResourceId as default (-1) and rely on higher-level code.
    if (RD->GetLODCount() > 0) {
        const LODResource& L0 = RD->GetLODResource(0);
        // no GPU resource ids in this prototype; Index ranges copied to proxy
        if (L0.GetNumSections() > 0) {
            const SectionInfo& S = L0.Sections[0];
            Proxy->SetGeometry(-1, -1, S.FirstIndex, S.NumIndices, S.MaterialIndex);
        }
    }

    return Proxy;
}
} // namespace Engine