#include "DefferedSceneRenderer.h"
#include "ScreenPass.h"
#include "StaticMeshProcess.h"
#include <iostream>
namespace Renderer {

    void DefferedSceneRenderer::Build(RenderCore::RenderGraphBuilder& graphBuilder)
    {
		auto SceneColorTargetTexture = SceneTextures.SceneColor;
        //±ærendererªÊ÷∆µΩDefferedOutputColor
        {
            
        }
        //addScreenPass(graphBuilder, SceneColorTargetTexture.get(), SceneColorTargetTexture.get());
        
    }
}