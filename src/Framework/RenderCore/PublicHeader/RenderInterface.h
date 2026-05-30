#pragma once

#include <string>
#include <vector>
#include "Module.h"
#include "SceneView.h"

namespace RenderCore {

class RENDERCORE_API RenderInterface : public Core::Module
{
public:
    virtual ~RenderInterface() = default;
    virtual void BeginRender(Engine::SceneViewFamily* sceneViewCollection) = 0;
    virtual Engine::SceneInterface* AllocateScene() = 0;
private:


};


}