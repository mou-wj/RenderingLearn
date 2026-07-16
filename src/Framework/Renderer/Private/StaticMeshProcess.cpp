#include "StaticMeshProcess.h"
#include "Scene.h"
#include "StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "StaticMeshProxy.h"
using namespace Engine;
namespace Renderer {
	void StaticMeshDrawBuild(const std::vector<Engine::StaticMeshProxy*>& meshs, MeshBatchList& outDrawMeshList)
	{
        for (const auto& mesh : meshs) {
            if (!mesh)
            {
                continue;
            }
            auto staticMesh = mesh->GetStaticMeshComponent()->GetStaticMesh();
            auto RenderData = staticMesh->GetRenderData();
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
                auto material = staticMesh->GetMaterial(Section.MaterialIndex);
                if (!material)
                {
                    continue;
                }

                MeshBatch& Batch =
                    outDrawMeshList.emplace_back();

                //------------------------------------------------
                // Batch-level state
                //------------------------------------------------

                Batch.VertexFactory =
                    LOD.VertexFactory.get();

                Batch.MaterialProxy =
                    material->GetRenderProxy();
				Batch.LocalToWorld = mesh->GetLocalToWorld();
                Batch.WorldToLocal = mesh->GetWorldToLocal();
                Batch.IndexBuffer =
                    LOD.IndexBuffer
                    .Buffer.get();


                Batch.LODIndex =
                    static_cast<uint8_t>(
                        LODIndex);

                //------------------------------------------------
                // Element-level draw range
                //------------------------------------------------

                auto& Element =
                    Batch.Elements.emplace_back();

                Element.PrimitiveUniformBufferData =
                    nullptr;

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
	}



}