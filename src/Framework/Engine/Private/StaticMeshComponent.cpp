// StaticMeshComponent.cpp

#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "StaticMeshProxy.h"
#include "Material.h"

namespace Engine
{

    StaticMeshComponent::StaticMeshComponent()
    {
    }

    void StaticMeshComponent::SetStaticMesh(StaticMesh* InMesh)
    {
        Mesh = std::move(InMesh);

        // Bounds may change
        UpdateWorldTransform();

        // Need recreate SceneProxy
        MarkRenderStateDirty();
    }

    void StaticMeshComponent::SetMaterial(
        uint32_t SlotIndex,
        MaterialInterface* Material)
    {
        if (SlotIndex >= OverrideMaterials.size())
        {
            OverrideMaterials.resize(SlotIndex + 1);
        }

        OverrideMaterials[SlotIndex] = Material;

        MarkRenderStateDirty();
    }

    MaterialInterface* StaticMeshComponent::GetMaterial(
        uint32_t SlotIndex) const
    {
        if (SlotIndex >= OverrideMaterials.size())
        {
            return nullptr;
        }

        return OverrideMaterials[SlotIndex];
    }

    PrimitiveSceneProxy*
        StaticMeshComponent::CreateSceneProxy() const
    {
        if (!Mesh)
        {
            return nullptr;
        }

        auto RenderData = Mesh->GetRenderData();

        if (!RenderData)
        {
            return nullptr;
        }

        std::vector<const MaterialRenderProxy*> MaterialProxies;
        MaterialProxies.reserve(OverrideMaterials.size());

        for (MaterialInterface* Material : OverrideMaterials)
        {
            if (Material)
            {
                MaterialProxies.push_back(
                    Material->GetRenderProxy());
            }
            else
            {
                MaterialProxies.push_back(nullptr);
            }
        }

        return new StaticMeshProxy(this);
    }

    Core::BoxSphereBounds
        StaticMeshComponent::CalcBounds(
            const FTransform& LocalToWorld) const
    {
        if (!Mesh)
        {
            return {};
        }

        auto RenderData = Mesh->GetRenderData();

        if (!RenderData)
        {
            return {};
        }

        return Core::BoxSphereBounds();
    }

} // namespace Engine