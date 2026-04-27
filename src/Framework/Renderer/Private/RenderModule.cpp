#include "RenderModule.h"
#include "DefferedSceneRenderer.h"
#include "Scene.h"
#include "RenderThread.h"
#include "ScreenPass.h"
namespace Renderer {

    SceneRendererSP CreateSceneRenderer() {
        return std::make_shared<DefferedSceneRenderer>();
    }

    RenderModule::RenderModule() = default;
    RenderModule::~RenderModule() = default;

    void RenderModule::StartupModule()
    {
        //创建渲染线程
        RenderCore::StartRenderThread();
		InitGlobalRenderResource();
        bLoaded = true;
    }

    void RenderModule::ShutdownModule()
    {
        bLoaded = false;
        // 关闭渲染线程
        RenderCore::StopRenderThread();
		ReleaseGlobalRenderResource();
    }

    bool RenderModule::IsLoaded() const
    {
        return bLoaded;
    }


    void RenderModule::BeginRender(
        Engine::SceneViewCollection* Views)
    {
        RenderCore::RenderGraphBuilder builder;
        auto rhiColorTex = Views->RenderTarget->RenderTarget;
        //auto targetTexture = builder.RegisterExternalTexture("RenderTarget", rhiColorTex);
        
        auto sceneRenderer = CreateSceneRenderer();
        //sceneRenderer->Scene =  dynamic_cast<Engine::Scene*>(Views->Scene);
        sceneRenderer->Views = Views;
        //sceneRenderer->SceneTextures.SceneColor = targetTexture;
        sceneRenderer->Build(builder);
        
        builder.Execute();

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