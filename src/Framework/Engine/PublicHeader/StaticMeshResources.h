
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <vector>
#include <memory>
#include "BoxSphereBounds.h"
#include "Material.h"
namespace Engine {
    // Forward placeholders for shader/texture types. In a real engine, replace
    // these with concrete types or resource handles managed by the RHI/resource system.
    struct ShaderProgram; // abstract shader program handle (opaque)
    struct Texture;       // abstract texture handle (opaque)
    class LocalVertexFactory;

    struct ENGINE_API VertexBuffer {
        std::vector<float> vertexs;
        uint64_t GetNumVertices() const { return vertexs.size(); }
        
    };
    

    // Lightweight index buffer: 32-bit indices
    struct ENGINE_API IndexBuffer {
        std::vector<uint32_t> Indices;

        size_t GetNumIndices() const { return Indices.size(); }
    };

    // Section describes a sub-range of the index buffer that uses a specific material.
    struct ENGINE_API SectionInfo {
        // first index in the index buffer (index into IndexBuffer::Indices)
        uint32_t FirstIndex = 0;
        // number of indices in this section
        uint32_t NumIndices = 0;
        // material index into the RenderData's Materials array
        int32_t MaterialIndex = -1;
    };

    // LOD resource contains one vertex buffer, one index buffer and multiple sections.
    struct ENGINE_API LODResource {
        // Vertex data is type-erased via IVertexData. Use VertexData<T> to supply
        // concrete typed vertex arrays (e.g., VertexData<StaticVertex>).
        std::vector<VertexBuffer> VertexBuffers;
        IndexBuffer Indices;        // per-LOD index data
        std::vector<SectionInfo> Sections; // sub-mesh sections mapped to materials

        // Convenience: query number of sections
        size_t GetNumSections() const { return Sections.size(); }
        // Convenience: get vertex count if VertexDataPtr is set
        size_t GetNumVertices() const { return VertexBuffers.empty() ? VertexBuffers[0].GetNumVertices() : 0; }
    };


    // FStaticMeshRenderData: top-level render resource container for a StaticMesh.
    // Contains multiple LODs, a material table, and bounds used for culling.
    class ENGINE_API FStaticMeshRenderData {
    public:

        FStaticMeshRenderData() = default;
        ~FStaticMeshRenderData() = default;

        // LOD resources (LOD0 = highest detail)
        std::vector<LODResource> LODResources;
		std::vector<LocalVertexFactory*> LODVertexFactories; // one per LOD, created from LODResources

        // Bounds (AABB) for frustum culling and coarse occlusion
        Core::AABB Bounds;

        // ----------------- Minimal public interfaces -----------------
        // Number of LODs available
        size_t GetLODCount() const { return LODResources.size(); }

        // Access a LOD resource (const, read-only for RenderThread)
        // Caller must ensure index < GetLODCount().
        const LODResource& GetLODResource(size_t Index) const { return LODResources[Index]; }

        // Get bounds for culling
        const Core::AABB& GetBounds() const { return Bounds; }


        // Utilities to append resources (used on GameThread during build/load)
        void AddLOD(LODResource&& LOD) { LODResources.emplace_back(std::move(LOD)); }

        // Intended usage: build on GameThread, then the RenderThread may read these
        // structures concurrently (read-only). Do not modify after submitting to the scene.

    private:
        // Non-copyable to avoid accidental copies of large buffers; allow move if needed.
        FStaticMeshRenderData(const FStaticMeshRenderData&) = delete;
        FStaticMeshRenderData& operator=(const FStaticMeshRenderData&) = delete;
    };
} // namespace NSRender