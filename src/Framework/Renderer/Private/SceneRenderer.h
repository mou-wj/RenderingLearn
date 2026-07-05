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
    struct ShadowRenderView
    {
        ShadowAllocationSlice Allocation;
        Core::Float4x4 ViewProjection;
        uint32_t TargetWidth = 0;
        uint32_t TargetHeight = 0;
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
        void BuildSceneLightShadowMap(RenderCore::RenderGraphBuilder& graphBuilder);
        void BuildSceneLightCascadeShadowMap(RenderCore::RenderGraphBuilder& graphBuilder,const Engine::SceneView& view);
        void UploadShadowMapInfo(RenderCore::RenderGraphBuilder& graphBuilder);
    protected:
		void BuildLightShadow(RenderCore::RenderGraphBuilder& graphBuilder , std::vector<ShadowRenderView>& shadowRenderViews);
    };
    using SceneRendererSP = std::shared_ptr<SceneRenderer>;


}