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
    explicit StaticMesh(std::shared_ptr<StaticMeshRenderData> InRenderData) : RenderData(std::move(InRenderData)) {}
    ~StaticMesh() = default;

    // Accessors
    size_t GetLODCount() const { return RenderData ? RenderData->GetLODCount() : 0; }
    const LODResource& GetLODResource(size_t Index) const { return RenderData->GetLODResource(Index); }
    const Core::AABB& GetBounds() const { static Core::AABB Empty; return RenderData ? RenderData->GetBounds() : Empty; }

    std::shared_ptr<StaticMeshRenderData> GetRenderData() const { return RenderData; }

private:
    std::shared_ptr<StaticMeshRenderData> RenderData; // shared ownership of underlying render data
};

using StaticMeshSP = std::shared_ptr<StaticMesh>;
} // namespace Engine