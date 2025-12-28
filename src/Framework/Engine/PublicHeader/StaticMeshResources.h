
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <vector>
#include <memory>
#include "BoxSphereBounds.h"
namespace Engine {
    // Forward placeholders for shader/texture types. In a real engine, replace
    // these with concrete types or resource handles managed by the RHI/resource system.
    struct ShaderProgram; // abstract shader program handle (opaque)
    struct Texture;       // abstract texture handle (opaque)


    // Vertex structure: position, normal, texcoord (UV0)
    struct StaticVertex {
        float Px, Py, Pz;    // position
        float Nx, Ny, Nz;    // normal
        float U, V;          // uv0
    };

    // Vertex data interface: abstract access to vertex array of arbitrary vertex type.
    // Implementations must be safe for read-only access on the RenderThread after
    // construction on the GameThread.
    struct ENGINE_API VertexDataInterface {
        virtual ~VertexDataInterface() = default;
        // Number of vertices
        virtual size_t GetNumVertices() const = 0;
        // Pointer to tightly-packed vertex array (type-erased). May be nullptr if empty.
        virtual const void* GetData() const = 0;
        // Stride in bytes of a single vertex element
        virtual size_t GetStride() const = 0;
    };

    // Templated concrete vertex data container. Stores a vector of vertex-type T.
    // Example: VertexData<StaticVertex> holds positions/normals/uvs.
    template<typename T>
    struct VertexData : public VertexDataInterface {
        std::vector<T> Vertices;

        VertexData() = default;
        explicit VertexData(std::vector<T>&& InVertices) : Vertices(std::move(InVertices)) {}

        size_t GetNumVertices() const override { return Vertices.size(); }
        const void* GetData() const override { return Vertices.empty() ? nullptr : &Vertices[0]; }
        size_t GetStride() const override { return sizeof(T); }

        // Utility to append or replace
        void SetVertices(std::vector<T>&& In) { Vertices = std::move(In); }
        const std::vector<T>& GetVertices() const { return Vertices; }
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
        std::unique_ptr<VertexDataInterface> VertexDataPtr;
        IndexBuffer Indices;        // per-LOD index data
        std::vector<SectionInfo> Sections; // sub-mesh sections mapped to materials

        // Convenience: query number of sections
        size_t GetNumSections() const { return Sections.size(); }
        // Convenience: get vertex count if VertexDataPtr is set
        size_t GetNumVertices() const { return VertexDataPtr ? VertexDataPtr->GetNumVertices() : 0; }
    };

    // MaterialProxy: minimal representation of a material for rendering purposes.
    // Holds references to shader program and textures (opaque pointers/handles).
    struct ENGINE_API MaterialProxy {
        ShaderProgram* Shader = nullptr; // shader program used by this material
        std::vector<Texture*> Textures;  // list of textures used by the material

        // Additional material flags or parameters can be added later.
    };

    // FStaticMeshRenderData: top-level render resource container for a StaticMesh.
    // Contains multiple LODs, a material table, and bounds used for culling.
    class ENGINE_API FStaticMeshRenderData {
    public:

        FStaticMeshRenderData() = default;
        ~FStaticMeshRenderData() = default;

        // LOD resources (LOD0 = highest detail)
        std::vector<LODResource> LODResources;

        // Materials referenced by sections. Material indices in SectionInfo index into this array.
        std::vector<MaterialProxy> Materials;

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

        // Get material proxy by index
        const MaterialProxy* GetMaterial(size_t Index) const {
            if (Index >= Materials.size()) return nullptr;
            return &Materials[Index];
        }

        // Utilities to append resources (used on GameThread during build/load)
        void AddLOD(LODResource&& LOD) { LODResources.emplace_back(std::move(LOD)); }
        int32_t AddMaterial(const MaterialProxy& M) { Materials.push_back(M); return (int32_t)Materials.size() - 1; }

        // Intended usage: build on GameThread, then the RenderThread may read these
        // structures concurrently (read-only). Do not modify after submitting to the scene.

    private:
        // Non-copyable to avoid accidental copies of large buffers; allow move if needed.
        FStaticMeshRenderData(const FStaticMeshRenderData&) = delete;
        FStaticMeshRenderData& operator=(const FStaticMeshRenderData&) = delete;
    };
} // namespace NSRender