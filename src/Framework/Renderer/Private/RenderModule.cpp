#include "RenderModule.h"
#include "DefferedSceneRenderer.h"
#include "ForwardSceneRenderer.h"
#include "Scene.h"
#include "RenderThread.h"
#include "ScreenPass.h"
#include "SceneInterface.h"
#include "Viewport.h"
#include "RenderResource.h"
#include "MeshMaterialShader.h"
#include "MateiralShader.h"
#include "GlobalShader.h"
#include "ShaderCompiler.h"
using namespace RenderCore;
using namespace Engine;

namespace Renderer {

    SceneRendererSP CreateSceneRenderer() {
        return std::make_shared<ForwardSceneRenderer>();
    }

    RenderModule::RenderModule() = default;
    RenderModule::~RenderModule() = default;

    void RenderModule::StartupModule()
    {
        //创建渲染线程
        RenderCore::StartRenderThread();
		InitGlobalRenderResource();
        GMeshMaterialShaderMap.Initialize();
        GMaterialShaderMap.Initialize();
        GShaderMap.Initialize();
        bLoaded = true;
    }

    void RenderModule::ShutdownModule()
    {
        bLoaded = false;
        GRenderTargetPool.Clear();
        GTransientResourceAllocator.ReleaseRHI();
		GMeshMaterialShaderMap.Clear();
        GMaterialShaderMap.Clear();
		GShaderMap.Clear();
        ReleaseGlobalRenderResource();
        // 关闭渲染线程
        RenderCore::StopRenderThread();
		
    }

    bool RenderModule::IsLoaded() const
    {
        return bLoaded;
    }


    void RenderModule::BeginRender(
        Engine::SceneViewFamily* Views)
    {
        RenderCore::RenderGraphBuilder builder;
        auto ColorTex = Views->RenderTarget->GetRenderTarget();
        auto sceneColorTexture = builder.RegisterExternalTexture("RenderTarget", ColorTex);
        PoolRenderTargetDesc depthDesc;
        depthDesc.Format = RHI::ERHIFormat::D32_Float;
        depthDesc.Width = ColorTex->GetRHI()->GetDesc().Width;
		depthDesc.Height = ColorTex->GetRHI()->GetDesc().Height;
        depthDesc.Usage = RHI::ERHITextureCreateFlag::DepthStencil;
        auto PoolDepthTarget = GRenderTargetPool.GetFreeRenderTarget(depthDesc);
		PoolDepthTarget->MarkUsed(true);
		auto depthText = builder.RegisterExternalTexture("DepthTarget", PoolDepthTarget.get());

        auto sceneRenderer = CreateSceneRenderer();
        sceneRenderer->Scene =  dynamic_cast<Scene*>(Views->Scene);
        sceneRenderer->Views = Views;
        sceneRenderer->SceneTextures.SceneColor = sceneColorTexture;
        sceneRenderer->SceneTextures.SceneDepth = depthText;
        sceneRenderer->Build(builder);
        
        builder.Execute();
        PoolDepthTarget->MarkUsed(false);
    }
    Engine::SceneInterface* RenderModule::AllocateScene() {
        return new Scene();
    }


	IMPLEMENT_SIMPLE_MODULE(RenderModule, "Renderer");

    RenderModule* GetRenderModuleInstance() {
        static RenderModule* instance = nullptr;
        if (instance == nullptr) {
            instance = dynamic_cast<RenderModule*>(Core::ModuleManager::Get().GetModule("Renderer").get());
        }
        return instance;
    }
}