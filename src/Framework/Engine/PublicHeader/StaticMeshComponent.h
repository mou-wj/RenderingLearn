// StaticMeshComponent.h
// Scene instance of a StaticMesh asset. Lives on GameThread and creates a SceneProxy for RenderThread.
#pragma once

#include "PrimitiveComponent.h"
#include <memory>
#include "EngineExport.h"
#include "Material.h"
namespace Engine {
class StaticMesh;

class ENGINE_API StaticMeshComponent : public PrimitiveComponent {
public:
    StaticMeshComponent();
    ~StaticMeshComponent() override;

    // Set or get the mesh asset
    void SetStaticMesh(std::shared_ptr<StaticMesh> InMesh) { Mesh = std::move(InMesh); }
    std::shared_ptr<StaticMesh> GetStaticMesh() const { return Mesh; }

    // Visibility/LOD
    void SetVisible(bool b) { bVisible = b; MarkRenderStateDirty(); }
    bool GetVisible() const { return bVisible; }

    void SetForcedLOD(int32_t LOD) { ForcedLOD = LOD; MarkRenderStateDirty(); }
    int32_t GetForcedLOD() const { return ForcedLOD; }

    // PrimitiveComponent overrides
    PrimitiveSceneProxy* CreateSceneProxy() const override;
    Core::BoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

private:
    std::shared_ptr<StaticMesh> Mesh; // asset pointer (not owning GPU resources)
    int32_t ForcedLOD;                // -1 = auto
    std::vector<MaterialInterfaceSP> Materials; // material interface pointers (not owning GPU resources)
};
} // namespace Engine