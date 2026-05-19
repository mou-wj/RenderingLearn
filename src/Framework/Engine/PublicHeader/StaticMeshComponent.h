// StaticMeshComponent.h
#pragma once

#include "PrimitiveComponent.h"
#include "StaticMesh.h"
#include "EngineExport.h"
#include <memory>
#include <vector>

namespace Engine
{
    class MaterialInterface;
    class StaticMesh;
    class StaticMeshProxy;

    class ENGINE_API StaticMeshComponent : public PrimitiveComponent
    {
        DEFINE_COMPONENT_TYPE(StaticMeshComponent)

    public:
        StaticMeshComponent();
        ~StaticMeshComponent() override = default;

        //----------------------------------------
        // Mesh
        //----------------------------------------

        void SetStaticMesh(StaticMesh* InMesh);
        StaticMesh* GetStaticMesh() const { return Mesh; }

        //----------------------------------------
        // Material Override
        //----------------------------------------

        void SetMaterial(uint32_t SlotIndex, MaterialInterface* Material);
        MaterialInterface* GetMaterial(uint32_t SlotIndex) const;

        uint32_t GetNumMaterials() const
        {
            return static_cast<uint32_t>(OverrideMaterials.size());
        }

        //----------------------------------------
        // LOD
        //----------------------------------------

        void SetForcedLOD(int32_t InLOD)
        {
            ForcedLOD = InLOD;
            MarkRenderStateDirty();
        }

        int32_t GetForcedLOD() const
        {
            return ForcedLOD;
        }

        //----------------------------------------
        // PrimitiveComponent Overrides
        //----------------------------------------

        PrimitiveSceneProxy* CreateSceneProxy() const override;

        Core::BoxSphereBounds CalcBounds(
            const FTransform& LocalToWorld) const override;

    private:
        // CPU-side mesh asset reference
        StaticMesh* Mesh;

        // Material overrides (same concept as UE OverrideMaterials)
        std::vector<MaterialInterface*> OverrideMaterials;

        // -1 = auto LOD
        int32_t ForcedLOD = -1;
    };

} // namespace Engine