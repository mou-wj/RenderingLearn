#pragma once

#include "StaticMesh.h"

namespace Engine {

extern StaticMeshSP GStaticMesh_Cube;
extern StaticMeshSP GStaticMesh_Sphere;
extern StaticMeshSP GStaticMesh_Plane;
extern StaticMeshSP GStaticMesh_Cylinder;

ENGINE_API bool InitializeShapeStaticMeshes();
ENGINE_API void ReleaseShapeStaticMeshes();

} // namespace Engine
