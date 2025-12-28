#pragma once

#include <string>
#include <vector>
#include "Module.h"
#include "FrameContext.h"
#include "SceneView.h"

namespace RenderCore {

class RenderInterface : public Core::Module
{
public:
    virtual ~RenderInterface() = default;
    virtual void BeginRender(const FrameContext& context,const Engine::SceneViewCollection& sceneViewCollection) = 0;
private:


};


}