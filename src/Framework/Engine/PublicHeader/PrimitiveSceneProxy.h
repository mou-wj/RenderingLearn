// PrimitiveSceneProxy.h
// RenderThread-only read-only representation of a PrimitiveComponent.
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <vector>
#include "BoxSphereBounds.h"
namespace Engine {
    // Forward declarations
    struct SceneView;

    using ResourceId = int32_t; // abstract handle for GPU resources managed elsewhere

    // MeshBatch: describes a single draw submission range (renderer will translate to actual RHI calls)
    struct MeshBatch {
        ResourceId VertexBufferId;
        ResourceId IndexBufferId;
        uint32_t IndexStart;
        uint32_t IndexCount;
        int32_t MaterialId;
        uint64_t SortKey; // for ordering
    };

    using MeshBatchList = std::vector<MeshBatch>;

    class ENGINE_API PrimitiveSceneProxy {
    public:
        PrimitiveSceneProxy(int32_t InPrimitiveId);
        virtual ~PrimitiveSceneProxy();

        // Identification & bounds
        int32_t GetPrimitiveId() const { return PrimitiveId; }
        const Core::AABB& GetBounds() const { return ProxyBounds; }

        // LocalToWorld matrix (row-major 4x4 stored in float[16])
        const float* GetLocalToWorld() const { return LocalToWorld; }

        // Visibility / state
        virtual bool IsVisible() const { return bVisible; }
        virtual bool CastsShadow() const { return bCastShadow; }
        virtual bool IsOpaque() const { return bOpaque; }
        virtual uint32_t GetRenderFlags() const { return RenderFlags; }

        // Renderer-facing query: fill mesh batches for this view.
        // Called on RenderThread. Must not touch GameThread data.
        virtual void GetMeshBatches(const SceneView& View, MeshBatchList& OutBatches) const = 0;

        // Hints
        virtual bool HasStaticGeometry() const = 0;
        virtual bool IsDynamic() const = 0;

        // Read-only resource access (IDs only)
        virtual ResourceId GetVertexBufferId() const { return VertexBufferId; }
        virtual ResourceId GetIndexBufferId()  const { return IndexBufferId; }
        virtual int32_t GetMaterialId(int32_t ElementIndex) const;

    protected:
        // Filled at construction time (RenderThread-only after creation)
        int32_t PrimitiveId;
        Core::AABB ProxyBounds;
        float LocalToWorld[16];
        ResourceId VertexBufferId;
        ResourceId IndexBufferId;
        std::vector<int32_t> MaterialIds;
        bool bVisible;
        bool bCastShadow;
        bool bOpaque;
        uint32_t RenderFlags;
    };

    // Simple StaticPrimitiveSceneProxy example: represents a single static mesh with one batch
    class ENGINE_API StaticPrimitiveSceneProxy : public PrimitiveSceneProxy {
    public:
        StaticPrimitiveSceneProxy(int32_t InPrimitiveId);
        ~StaticPrimitiveSceneProxy() override;

        void GetMeshBatches(const SceneView& View, MeshBatchList& OutBatches) const override;
        bool HasStaticGeometry() const override { return true; }
        bool IsDynamic() const override { return false; }

        // Simple setter to populate proxy (used by CreateSceneProxy on GameThread)
        void SetGeometry(ResourceId VB, ResourceId IB, uint32_t IndexStart, uint32_t IndexCount, int32_t MaterialId);

    private:
        uint32_t BatchIndexStart;
        uint32_t BatchIndexCount;
        int32_t BatchMaterialId;
    };
} // namespace Engine