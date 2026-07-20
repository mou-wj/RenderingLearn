#include "SlateRHIModule.h"
#include "SlateRHIRenderer.h"
namespace SlateRHIRenderer {
    SlateRHIRendererModule* GSlateRHIRendererModule = nullptr;
    SlateRHIRendererModule::SlateRHIRendererModule():Renderer(nullptr) {
        GSlateRHIRendererModule = this;
    }
    void SlateRHIRendererModule::StartupModule()
    {
        bLoaded = true;
    }

    void SlateRHIRendererModule::ShutdownModule()
    {
        delete Renderer;
        bLoaded = false;
    }

    bool SlateRHIRendererModule::IsLoaded() const
    {
        return bLoaded;
    }

    SlateCore::SlateRenderer* SlateRHIRendererModule::CreateSlateRenderer()
    {
        // UE 风格：只创建一次
        if (!Renderer)
        {
            Renderer = new SlateRHIRenderer();
        }

        return Renderer;
    }
    IMPLEMENT_SIMPLE_MODULE(SlateRHIRendererModule, "SlateRHIRenderer")
}