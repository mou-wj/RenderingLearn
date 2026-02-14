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
    virtual void BeginRender(Engine::SceneViewCollection* sceneViewCollection) = 0;
private:


};


}