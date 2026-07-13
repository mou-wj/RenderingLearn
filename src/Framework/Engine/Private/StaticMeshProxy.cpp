// StaticMeshProxy.cpp
#include "StaticMeshProxy.h"
#include "SceneView.h"
#include "StaticMeshComponent.h"
#include "Transform.hpp"
#include <cstring>
namespace Engine {


StaticMeshProxy::StaticMeshProxy(const StaticMeshComponent* InComponent):MeshComponent(InComponent)
{
    LocalToWorld = Core::MakeTranslationMatrix(InComponent->GetLocalLocation());
    WorldToLocal = Core::Inverse(LocalToWorld);
}

StaticMeshProxy::~StaticMeshProxy() {}

} // namespace Engine