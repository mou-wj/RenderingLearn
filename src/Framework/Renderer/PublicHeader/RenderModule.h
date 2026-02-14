#pragma once
#include "RenderInterface.h"
#include "SceneView.h"
namespace Renderer {


    class RENDERER_API RenderModule final : public RenderCore::RenderInterface
    {
    public:
        RenderModule();
        ~RenderModule() override;

        // Module
        void StartupModule() override;
        void ShutdownModule() override;
        bool IsLoaded() const override;

        // RenderInterface
        void BeginRender(Engine::SceneViewCollection* Views) override;

    private:

    private:
        bool bLoaded = false;
        std::string ModuleName = "RenderModule";
    };


    RENDERER_API RenderModule* GetRenderModuleInstance();






}