#pragma once
#include "SlateRenderer.h"
namespace SlateRHIRenderer {
    class SLATERHIRENDERER_API SlateRHIRendererModule final : public SlateCore::SlateRendererModule
    {
    public:
        SlateRHIRendererModule();
        virtual void StartupModule() override;
        virtual void ShutdownModule() override;
        virtual bool IsLoaded() const override;

        virtual SlateCore::SlateRenderer* CreateSlateRenderer() override;

    private:
        bool bLoaded = false;

        // Module 内部持有
        SlateCore::SlateRenderer* Renderer;
    };

    extern SLATERHIRENDERER_API SlateRHIRendererModule* GSlateRHIRendererModule;
}