#pragma once

#include <vector>
#include <set>
#include "Math.hpp"
#include "EngineExport.h"
#include "TypeIDCast.h"

namespace Engine
{
	class SceneInterface;
    class ENGINE_API SceneComponent
    {
    public:
        SceneComponent();
        virtual ~SceneComponent();
        DECLARE_TYPE_ID_BASE_TYPE(SceneComponent)
    public:
        void AttachTo(
            SceneComponent* parent,
            bool keepWorldTransform = false);

        void Detach(
            bool keepWorldTransform = false);

        SceneComponent* GetAttachParent() const;

        const std::vector<SceneComponent*>&
            GetAttachChildren() const;

		void AddSceneListener(SceneInterface* listener);
        void RemoveSceneListener(SceneInterface* listener);

    public:
        void SetLocalLocation(
            const Core::Float3& location);

        void SetLocalRotation(
            const Core::Float3& rotation);

        void SetLocalScale(
            const Core::Float3& scale);

        void SetWorldLocation(
            const Core::Float3& location);

        void SetWorldRotation(
            const Core::Float3& rotation);

        void SetWorldScale(
            const Core::Float3& scale);

    public:
        const Core::Float3&
            GetLocalLocation() const;

        const Core::Float3&
            GetLocalRotation() const;

        const Core::Float3&
            GetLocalScale() const;

        Core::Float3 GetWorldLocation() const;
        Core::Float3 GetWorldRotation() const;
        Core::Float3 GetWorldScale() const;

    public:
        const Core::Mat4&
            GetComponentTransform();

        Core::Float3 GetForwardVector() const;
        Core::Float3 GetRightVector();
        Core::Float3 GetUpVector();

    public:
        virtual void MarkRenderStateDirty();

    protected:
        virtual void OnTransformChanged();

        void MarkTransformDirty();

        void UpdateTransformIfDirty() const;

        void UpdateWorldTransform() const;

    protected:
        Core::Float3 LocalLocation =
            Core::Float3(0.0f, 0.0f, 0.0f);

        Core::Float3 LocalRotation =
            Core::Float3(0.0f, 0.0f, 0.0f);

        Core::Float3 LocalScale =
            Core::Float3(1.0f, 1.0f, 1.0f);

        mutable Core::Mat4 WorldTransform =
            Core::Mat4::Identity();

        mutable bool bTransformDirty =
            true;

    protected:
        SceneComponent* Parent =
            nullptr;

        std::vector<SceneComponent*>
            Children;
        std::set<SceneInterface*> SceneListeners;
    };
}