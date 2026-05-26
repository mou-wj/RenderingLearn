// StaticMeshProxy.cpp
#include "StaticMeshProxy.h"
#include "SceneView.h"
#include "StaticMeshComponent.h"

#include <cstring>
namespace Engine {


StaticMeshProxy::StaticMeshProxy(const StaticMeshComponent* InComponent):MeshComponent(InComponent)
{
    
}

StaticMeshProxy::~StaticMeshProxy() {}

void StaticMeshProxy::GetMeshBatches(const SceneView& /*View*/, MeshBatchList& OutBatches) const {
    if (!MeshComponent)
    {
        return;
    }
    auto RenderData = MeshComponent->GetStaticMesh()->GetRenderData();
    if (RenderData->GetLODCount() == 0)
    {
        return;
    }

    // TODO:
    // 后续根据 distance/screen size
    // 选择 LOD
    const uint32_t LODIndex = 0;

    const LODResource& LOD =
        RenderData->GetLODResource(
            LODIndex);

    // 没有 VertexFactory 无法绘制
    if (!LOD.VertexFactory)
    {
        return;
    }

    for (const SectionInfo& Section :
        LOD.Sections)
    {
        // material index invalid
        auto material = MeshComponent->GetStaticMesh()->GetMaterial(Section.MaterialIndex);
        if (!material)
        {
            continue;
        }

        MeshBatch& Batch =
            OutBatches.emplace_back();

        //------------------------------------------------
        // Batch-level state
        //------------------------------------------------

        Batch.VertexFactory =
            LOD.VertexFactory.get();

        Batch.MaterialProxy =
            material->GetRenderProxy();

        Batch.IndexBuffer =
            LOD.IndexBuffer
            .Buffer.get();

        Batch.CastShadow =
            bCastShadow;

        Batch.bUseForDepthPass =
            bOpaque;

        Batch.ReverseCulling =
            false;

        Batch.LODIndex =
            static_cast<uint8_t>(
                LODIndex);

        //------------------------------------------------
        // Element-level draw range
        //------------------------------------------------

        auto& Element =
            Batch.Elements.emplace_back();

        Element.PrimitiveUniformBufferData =
            PrimitiveUniformBufferData;

        Element.FirstIndex =
            Section.FirstIndex;

        Element.BaseVertexIndex =
            Section.BaseVertexIndex;

        Element.NumIndices =
            Section.NumIndices;

        Element.NumInstances =
            1;
    }
}
} // namespace Engine