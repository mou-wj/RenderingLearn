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
        //camera.SetPosition({ 0.0f, 0.0f, -5.0f });
        //camera.SetTarget({ 0.0f, 0.0f, 0.0f });
        //camera.SetUp({ 0.0f, 1.0f, 0.0f });
        //camera.SetPerspective(Core::DegToRad(45.0f), texDesc.Width / (float)texDesc.Height, 0.1f, 100.0f);
        //auto view = camera.GetViewMatrix();
        //auto proj = camera.GetProjectionMatrix();
        ////绘制场景
        //SceneView sceneView;
        //sceneView.CameraWorldPos = camera.GetPosition();
        //sceneView.ViewMatrix = view;
        //sceneView.ProjectionMatrix = proj;
        //sceneView.ViewProjectionMatrix = view * proj;
        //SceneViewFamily sceneViewFamily;
        //sceneViewFamily.Scene = scene;
        //sceneViewFamily.AddView(sceneView);
        //RenderTarget target;
        //target.RenderTarget = renderTarget->GetRHI();
        //sceneViewFamily.RenderTarget = &target;
    }



}