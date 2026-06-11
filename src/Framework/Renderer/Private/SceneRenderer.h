#pragma once 
#include "SceneView.h"
#include "Scene.h"
#include "RenderGraphBuilder.h"
#include "IBLPrecomputeShader.h"
namespace Renderer {

    struct SceneTextures
    {
       
        // �����
        RenderCore::RenderGraphTextureRef SceneColor;
        RenderCore::RenderGraphTextureRef SceneDepth;

        // GBuffer����ѡ��������չ��
        RenderCore::RenderGraphTextureRef GBufferA;
        RenderCore::RenderGraphTextureRef GBufferB;
        RenderCore::RenderGraphTextureRef GBufferC;

        // ��ѡ������ / ��չ���
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

        // һ֡��Ⱦ���
        virtual void Build(RenderCore::RenderGraphBuilder& graphBuilder) = 0;

    protected:
        
    };
    using SceneRendererSP = std::shared_ptr<SceneRenderer>;


}