#pragma once 
#include "SceneView.h"
#include "Scene.h"
#include "RenderGraphBuilder.h"
namespace Renderer {

    struct SceneTextures
    {
       
        // 主输出
        RenderCore::RenderGraphTextureRef SceneColor;
        RenderCore::RenderGraphTextureRef SceneDepth;

        // GBuffer（可选，后续扩展）
        RenderCore::RenderGraphTextureRef GBufferA;
        RenderCore::RenderGraphTextureRef GBufferB;
        RenderCore::RenderGraphTextureRef GBufferC;

        // 可选：后处理 / 扩展输出
        RenderCore::RenderGraphTextureRef Velocity;
        RenderCore::RenderGraphTextureRef CustomDepth;

        uint32_t Width = 0;
        uint32_t Height = 0;

        void Reset()
        {

        }
    };

    // SceneRenderer.h
    class SceneRenderer
    {
    public:
        Scene* Scene;
        Engine::SceneViewFamily* Views;
        SceneTextures SceneTextures;

        virtual ~SceneRenderer() = default;

        // 一帧渲染入口
        virtual void Build(RenderCore::RenderGraphBuilder& graphBuilder) = 0;


    protected:
        
    };
    using SceneRendererSP = std::shared_ptr<SceneRenderer>;


}