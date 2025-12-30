// StaticMesh.h
// Minimal StaticMesh asset wrapper holding render data (LODs, sections, materials, bounds).
#pragma once
#include "EngineExport.h"
#include "StaticMeshResources.h"
#include <memory>
namespace Engine {
// Forward
class PrimitiveSceneProxy;

class ENGINE_API StaticMesh {
public:
    StaticMesh() = default;
    explicit StaticMesh(std::shared_ptr<FStaticMeshRenderData> InRenderData) : RenderData(std::move(InRenderData)) {}
    ~StaticMesh() = default;

    // Accessors
    size_t GetLODCount() const { return RenderData ? RenderData->GetLODCount() : 0; }
    const LODResource& GetLODResource(size_t Index) const { return RenderData->GetLODResource(Index); }
    const Core::AABB& GetBounds() const { static Core::AABB Empty; return RenderData ? RenderData->GetBounds() : Empty; }

    std::shared_ptr<FStaticMeshRenderData> GetRenderData() const { return RenderData; }

    // Create a SceneProxy for this mesh. Returned pointer is heap-allocated and
    // ownership is transferred to the caller (Scene/RenderThread).
    virtual PrimitiveSceneProxy* CreateSceneProxy() const;

private:
    std::shared_ptr<FStaticMeshRenderData> RenderData; // shared ownership of underlying render data
};
} // namespace Engine