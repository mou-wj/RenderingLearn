#pragma once

#include <string>
#include <vector>
#include "Module.h"
#include "SceneView.h"

namespace RenderCore {
class RenderTexture;
class RENDERCORE_API RenderInterface : public Core::Module
{
public:
    virtual ~RenderInterface() = default;
    virtual void BeginRender(Engine::SceneViewFamily* sceneViewCollection) = 0;
    virtual Engine::SceneInterface* AllocateScene() = 0;
    virtual void PreComputeIBL(RenderTexture* InHDRTexture,RenderTexture* OutDiffuseIBL,RenderTexture* OutSpecularIBL) = 0;
private:


};


}