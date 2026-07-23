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
        void BeginRender(Engine::SceneViewFamily* Views) override;
        Engine::SceneInterface* AllocateScene() override;
        void PreComputeIBL(RenderCore::RenderTexture* InHDRTexture, RenderCore::RenderTexture* OutDiffuseIBL, RenderCore::RenderTexture* OutSpecularIBL) override;
        bool PreComputePrimitiveSDF(Engine::PrimitiveComponent* PrimitiveComponent) override;

    private:
        void InitIBLLut();
    private:
        bool bLoaded = false;
        std::string ModuleName = "RenderModule";
    };





}