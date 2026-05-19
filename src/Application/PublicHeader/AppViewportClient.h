#pragma once
#include "SceneView.h"
#include "Viewport.h"
#include "ViewportClient.h"
namespace App {

    class APPLICATION_API AppViewportClient : public Engine::ViewportClient
    {
    public:
        AppViewportClient();
        virtual ~AppViewportClient();

        // ViewportClient ºËÐÄ½Ó¿Ú
        void Draw(Engine::Viewport* InViewport) override;

    private:
        void BuildSceneViews(Engine::Viewport* InViewport, Engine::SceneViewFamily& OutViews);

    private:

    };


}