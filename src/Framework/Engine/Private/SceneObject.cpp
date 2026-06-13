#include "SceneObject.h"

namespace Engine
{
    SceneObject::SceneObject() = default;

    SceneObject::SceneObject(SceneComponent* rootComponent)
        : RootComponent(rootComponent)
    {
    }

    SceneObject::~SceneObject() = default;

    SceneComponent* SceneObject::GetRootComponentRaw() const
    {
        return RootComponent;
    }

    void SceneObject::SetRootComponent(SceneComponent* rootComponent)
    {
        RootComponent = rootComponent;
    }

    void SceneObject::ResetRootComponent()
    {
        RootComponent = nullptr;
    }

    bool SceneObject::HasRootComponent() const
    {
        return RootComponent != nullptr;
    }
}
