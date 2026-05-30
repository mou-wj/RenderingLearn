#include "AppViewportClient.h"
#include "Module.h"
#include "RenderModule.h"
#include "Math.hpp"
namespace App {
    using namespace Engine;
    using namespace Core;
    using namespace RenderCore;

    AppViewportClient::AppViewportClient() = default;
    AppViewportClient::~AppViewportClient() = default;

    void AppViewportClient::InitResources()
    {
        AssetManager::Get().LoadSync<MaterialAsset>(Core::GetProjectDir() + "/resources/material/DefaultWhite/material.json");

        staticMeshAsset = AssetManager::Get().LoadSync<StaticMeshAsset>(Core::GetProjectDir() + "/resources/glb/sphere.glb");
        staticMeshComponent = new StaticMeshComponent();
        staticMeshComponent->SetStaticMesh(staticMeshAsset->GetMesh());
        scene = Renderer::GetRenderModuleInstance()->AllocateScene();
        scene->AddPrimitive(staticMeshComponent);
        scene->FlushPendingUpdates();

        camera.SetPosition({ 0.0f, 0.0f, -5.0f });
        camera.SetTarget({ 0.0f, 0.0f, 0.0f });
        camera.SetUp({ 0.0f, 1.0f, 0.0f });
        camera.SetPerspective(Core::DegToRad(45.0f), 1, 0.1f, 100.0f);
    }

    void AppViewportClient::ReleaseResources()
    {
		if (scene) {
			scene->RemovePrimitive(staticMeshComponent);
            scene->FlushPendingUpdates();
		}
		delete staticMeshComponent;
		staticMeshComponent = nullptr;
		staticMeshAsset.reset();
        delete scene;
		scene = nullptr;
        AssetManager::Get().Clear();
    }

    void AppViewportClient::Draw(Viewport* InViewport)
    {
        if (!InViewport)
            return;

        // 构建 SceneViewCollection
        ;
        family.RenderTarget = InViewport;
        BuildSceneViews(InViewport, family);

        // 调用 RenderInterface 执行渲染
        Renderer::GetRenderModuleInstance()->BeginRender(&family);
    }

    void AppViewportClient::BuildSceneViews(Engine::Viewport* InViewport, Engine::SceneViewFamily& OutViews)
    {

        auto view = camera.GetViewMatrix();
        auto proj = camera.GetProjectionMatrix();
		auto vp = view * proj;
        ;
        //绘制场景
        SceneView sceneView;
        sceneView.CameraWorldPos = camera.GetPosition();
        sceneView.ViewMatrix = view;
        sceneView.ProjectionMatrix = proj;
        sceneView.ViewProjectionMatrix = proj * view;
        sceneView.Viewport.width = InViewport->GetWidth();
        sceneView.Viewport.height = InViewport->GetHeight();
        family.Scene = scene;
        family.ClearViews();
        family.AddView(sceneView);
    }
    bool AppViewportClient::
        OnMouseButton(
            const Slate::MouseButtonEvent&
            Event)
    {
        const bool pressed =
            Event.Event ==
            Slate::EInputEvent
            ::Pressed;

        switch (Event.Button)
        {
            case Slate::EMouseButton
            ::Right:
            {
                bRightMouseDown =
                    pressed;

                return true;
            }

            case Slate::EMouseButton
            ::Middle:
            {
                bMiddleMouseDown =
                    pressed;

                return true;
            }
        }

        return false;
    }

    bool AppViewportClient::
        OnMouseMove(
            const Slate::MouseMoveEvent&
            Event)
    {
        constexpr float
            RotateSpeed =
            0.005f;

        constexpr float
            PanSpeed =
            0.005f;

        if (bRightMouseDown)
        {
            Yaw +=
                Event.DeltaX *
                RotateSpeed;

            Pitch -=
                Event.DeltaY *
                RotateSpeed;

            Pitch =
                std::clamp(
                    Pitch,
                    -1.5f,
                    1.5f);

            UpdateCameraTransform();

            return true;
        }
        
        if (bMiddleMouseDown)
        {
            auto position =
                camera.GetPosition();

            auto target =
                camera.GetTarget();

            Float3
                forward =
                Core::Normalize(
                    target -
                    position);

            const auto
                right =
                Core::Normalize(
                    Core::Cross(
                        forward,
                        camera.GetUp()));

            const auto
                up =
                Core::Normalize(
                    Core::Cross(
                        right,
                        forward));

            const auto offset =
                (-right *
                    Event.DeltaX +
                    up *
                    Event.DeltaY)
                * PanSpeed *
                CameraDistance;

            position +=
                offset;

            target +=
                offset;

            camera.SetPosition(
                position);

            camera.SetTarget(
                target);

            return true;
        }
        /**/
        return false;
    }
    bool AppViewportClient::
        OnMouseWheel(
            const Slate::MouseWheelEvent&
            Event)
    {
        constexpr float
            ZoomSpeed =
            0.25f;

        CameraDistance -=
            Event.Delta *
            ZoomSpeed;

        CameraDistance =
            CORE_MAX(
                CameraDistance,
                0.1f);

        UpdateCameraTransform();

        return true;
    }
    void AppViewportClient::
        UpdateCameraTransform()
    {
        const float cosPitch =
            std::cos(Pitch);

        Core::Float3 forward;
		forward = camera.GetTarget() - camera.GetPosition();

        forward.x =
            std::cos(Yaw) *
            cosPitch;
        
        forward.y =
            std::sin(Pitch);
        
        forward.z =
            std::sin(Yaw) *
            cosPitch;

        forward =
            Core::Normalize(
                forward);

        const auto target =
            camera.GetTarget();

        const auto position =
            target -
            forward *
            CameraDistance;

        camera.SetPosition(
            position);

        camera.SetUp(
            { 0.0f, 1.0f, 0.0f });
        camera.SetPerspective(Core::DegToRad(45.0f), 1, 0.1f, 100.0f);
    }



}