// StaticMeshProxy.cpp
#include "StaticMeshProxy.h"
#include "SceneView.h"
#include "StaticMeshComponent.h"

#include <cstring>
namespace Engine {


StaticMeshProxy::StaticMeshProxy(const StaticMeshComponent* InComponent):MeshComponent(InComponent)
{
    
}

StaticMeshProxy::~StaticMeshProxy() {}

} // namespace Engine