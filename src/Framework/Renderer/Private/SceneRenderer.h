#pragma once 
#include "SceneView.h"
#include "Scene.h"
#include "RenderGraphBuilder.h"
namespace Renderer {

    struct SceneTextures
    {
       
        // 主输出
        RenderCore::RenderGraphTextureSP SceneColor;
        RenderCore::RenderGraphTextureSP SceneDepth;

        // GBuffer（可选，后续扩展）
        RenderCore::RenderGraphTextureSP GBufferA;
        RenderCore::RenderGraphTextureSP GBufferB;
        RenderCore::RenderGraphTextureSP GBufferC;

        // 可选：后处理 / 扩展输出
        RenderCore::RenderGraphTextureSP Velocity;
        RenderCore::RenderGraphTextureSP CustomDepth;

        uint32_t Width = 0;
        uint32_t Height = 0;

        void Reset()
        {
            SceneColor.reset();
            SceneDepth.reset();
            GBufferA.reset();
            GBufferB.reset();
            GBufferC.reset();
            Velocity.reset();
            CustomDepth.reset();
        }
    };

    // SceneRenderer.h
    class SceneRenderer
    {
    public:
        Engine::Scene* Scene;
        Engine::SceneViewCollection* Views;
        SceneTextures SceneTextures;

        virtual ~SceneRenderer() = default;

        // 一帧渲染入口
        virtual void Build(RenderCore::RenderGraphBuilder& graphBuilder) = 0;


    protected:
        
    };
    using SceneRendererSP = std::shared_ptr<SceneRenderer>;


}