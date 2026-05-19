#include "AppViewportClient.h"
#include "Module.h"
#include "RenderModule.h"
namespace App {
    using namespace Engine;
    using namespace Core;
    using namespace RenderCore;

    AppViewportClient::AppViewportClient() = default;
    AppViewportClient::~AppViewportClient() = default;

    void AppViewportClient::Draw(Viewport* InViewport)
    {
        if (!InViewport)
            return;

        // 构建 SceneViewCollection
        SceneViewFamily family;
        family.RenderTarget = InViewport;
        BuildSceneViews(InViewport, family);

        // 调用 RenderInterface 执行渲染
        Renderer::GetRenderModuleInstance()->BeginRender(&family);
    }

    void AppViewportClient::BuildSceneViews(Engine::Viewport* InViewport, Engine::SceneViewFamily& OutViews)
    {
        SceneView view;
        OutViews.AddView(view);
    }



}