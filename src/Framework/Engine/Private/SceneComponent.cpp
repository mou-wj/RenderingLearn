#include "SceneComponent.h"
#include "Transform.hpp"
#include <algorithm>
#include "SceneInterface.h"

namespace Engine
{
    SceneComponent::SceneComponent()
    {
    }

    SceneComponent::~SceneComponent()
    {
        Detach();

        for (SceneComponent* child : Children)
        {
            child->Parent = nullptr;
        }
    }

    void SceneComponent::AttachTo(
        SceneComponent* parent,
        bool keepWorldTransform)
    {
        if (Parent == parent)
        {
            return;
        }

        Core::Mat4 worldTransform =
            GetComponentTransform();

        Detach(false);

        Parent = parent;

        if (Parent)
        {
            Parent->Children.push_back(this);
        }

        if (keepWorldTransform)
        {
            Core::Mat4 localTransform =
                worldTransform;

            if (Parent)
            {
                localTransform =
                    Core::Inverse(
                        Parent->GetComponentTransform())
                    * worldTransform;
            }

            LocalLocation =
                Core::GetTranslation(
                    localTransform);

            LocalRotation =
                Core::GetEulerRotation(
                    localTransform);

            LocalScale =
                Core::GetScale(
                    localTransform);
        }

        MarkTransformDirty();
    }

    void SceneComponent::Detach(
        bool keepWorldTransform)
    {
        if (!Parent)
        {
            return;
        }

        Core::Mat4 worldTransform =
            GetComponentTransform();

        auto& siblings =
            Parent->Children;

        siblings.erase(
            std::remove(
                siblings.begin(),
                siblings.end(),
                this),
            siblings.end());

        Parent = nullptr;

        if (keepWorldTransform)
        {
            LocalLocation =
                Core::GetTranslation(
                    worldTransform);

            LocalRotation =
                Core::GetEulerRotation(
                    worldTransform);

            LocalScale =
                Core::GetScale(
                    worldTransform);
        }

        MarkTransformDirty();
    }

    SceneComponent*
        SceneComponent::GetAttachParent() const
    {
        return Parent;
    }

    const std::vector<SceneComponent*>&
        SceneComponent::GetAttachChildren() const
    {
        return Children;
    }

    void SceneComponent::SetSceneOwner(SceneInterface* owner)
    {
        SceneOwner = owner;
    }

    SceneInterface* SceneComponent::GetSceneOwner() const
    {
        if (SceneOwner)
        {
            return SceneOwner;
        }

        if (Parent)
        {
            return Parent->GetSceneOwner();
        }

        return nullptr;
    }

    void SceneComponent::SetLocalLocation(
        const Core::Float3& location)
    {
        LocalLocation = location;
        MarkTransformDirty();
    }

    void SceneComponent::SetLocalRotation(
        const Core::Float3& rotation)
    {
        LocalRotation = rotation;
        MarkTransformDirty();
    }

    void SceneComponent::SetLocalScale(
        const Core::Float3& scale)
    {
        LocalScale = scale;
        MarkTransformDirty();
    }

    void SceneComponent::SetWorldLocation(
        const Core::Float3& location)
    {
        if (!Parent)
        {
            LocalLocation = location;
        }
        else
        {
            Core::Mat4 parentInverse =
                Core::Inverse(
                    Parent->GetComponentTransform());

            LocalLocation =
                Core::TransformPoint(
                    parentInverse,
                    location);
        }

        MarkTransformDirty();
    }

    void SceneComponent::SetWorldRotation(
        const Core::Float3& rotation)
    {
        LocalRotation = rotation;
        MarkTransformDirty();
    }

    void SceneComponent::SetWorldScale(
        const Core::Float3& scale)
    {
        LocalScale = scale;
        MarkTransformDirty();
    }

    const Core::Float3&
        SceneComponent::GetLocalLocation() const
    {
        return LocalLocation;
    }

    const Core::Float3&
        SceneComponent::GetLocalRotation() const
    {
        return LocalRotation;
    }

    const Core::Float3&
        SceneComponent::GetLocalScale() const
    {
        return LocalScale;
    }

    Core::Float3
        SceneComponent::GetWorldLocation() const
    {
        UpdateTransformIfDirty();

        return Core::GetTranslation(
            WorldTransform);
    }

    Core::Float3
        SceneComponent::GetWorldRotation() const
    {
        UpdateTransformIfDirty();

        return Core::GetEulerRotation(
            WorldTransform);
    }

    Core::Float3
        SceneComponent::GetWorldScale() const
    {
        UpdateTransformIfDirty();

        return Core::GetScale(
            WorldTransform);
    }

    const Core::Mat4&
        SceneComponent::GetComponentTransform()
    {
        UpdateTransformIfDirty();

        return WorldTransform;
    }

    Core::Float3
        SceneComponent::GetForwardVector() const
    {
        UpdateTransformIfDirty();

        return Core::Normalize(
            Core::TransformVector(
                WorldTransform,
                Core::Float3(
                    1.0f,
                    0.0f,
                    0.0f)));
    }

    Core::Float3
        SceneComponent::GetRightVector()
    {
        UpdateTransformIfDirty();

        return Core::Normalize(
            Core::TransformVector(
                WorldTransform,
                Core::Float3(
                    0.0f,
                    1.0f,
                    0.0f)));
    }

    Core::Float3
        SceneComponent::GetUpVector()
    {
        UpdateTransformIfDirty();

        return Core::Normalize(
            Core::TransformVector(
                WorldTransform,
                Core::Float3(
                    0.0f,
                    0.0f,
                    1.0f)));
    }

    void SceneComponent::MarkRenderStateDirty()
    {
        if (auto* owner = GetSceneOwner())
        {
            owner->NotifyComponentChanged(this);
        }
    }

    void SceneComponent::OnTransformChanged()
    {
        MarkRenderStateDirty();
    }

    void SceneComponent::MarkTransformDirty()
    {
        bTransformDirty = true;

        for (SceneComponent* child : Children)
        {
            child->MarkTransformDirty();
        }

        OnTransformChanged();
    }

    void SceneComponent::UpdateTransformIfDirty() const
    {
        if (!bTransformDirty)
        {
            return;
        }

        UpdateWorldTransform();
    }

    void SceneComponent::UpdateWorldTransform() const
    {
        Core::Mat4 localTransform =
            Core::MakeTransformMatrix(
                LocalLocation,
                LocalRotation,
                LocalScale);

        if (Parent)
        {
            WorldTransform =
                Parent->GetComponentTransform()
                * localTransform;
        }
        else
        {
            WorldTransform =
                localTransform;
        }

        bTransformDirty =
            false;
    }
}