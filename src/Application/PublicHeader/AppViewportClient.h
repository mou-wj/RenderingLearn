#pragma once
#include "SceneView.h"
#include "Viewport.h"
#include "ViewportClient.h"
#include "Camera.h"
#include "SceneInterface.h"
#include "AssetManager.h"
#include "StaticMeshComponent.h"
#include "SceneView.h"
namespace App {

    class APPLICATION_API AppViewportClient : public Engine::ViewportClient
    {
    public:
        AppViewportClient();
        virtual ~AppViewportClient();
        void InitResources();
        void ReleaseResources();

        // ViewportClient ºËÐÄ½Ó¿Ú
        void Draw(Engine::Viewport* InViewport) override;
    public:
        bool OnMouseButton(
            const Slate::MouseButtonEvent&
            Event) override;

        bool OnMouseMove(
            const Slate::MouseMoveEvent&
            Event) override;

        bool OnMouseWheel(
            const Slate::MouseWheelEvent&
            Event) override;
    private:
        void BuildSceneViews(Engine::Viewport* InViewport, Engine::SceneViewFamily& OutViews);
        void UpdateCameraTransform();
    private:
        bool bRightMouseDown =
            false;

        bool bMiddleMouseDown =
            false;

        float CameraDistance =
            5.0f;

        float Yaw =
            0.0f;

        float Pitch =
            0.0f;

    private:
        Engine::Camera camera;
        Engine::SceneInterface* scene;
        std::shared_ptr<Engine::StaticMeshAsset> staticMeshAsset;
        Engine::StaticMeshComponent* staticMeshComponent;
        Engine::SceneViewFamily family;
    };


}