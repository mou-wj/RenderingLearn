#pragma once
#include "Module.h"
#include "SlateViewport.h"
#include "Window.h"
namespace SlateCore {

    class SLATECORE_API SlateRenderer {
    public:
        virtual ~SlateRenderer() = default;
        virtual void Render(Window* window) = 0;
        virtual void CreateViewport(Window* window) = 0;

    };
	using SlateRendererSP = std::shared_ptr<SlateRenderer>;

    class SLATECORE_API SlateRendererModule : public Core::Module {
    public:
        virtual SlateRenderer* CreateSlateRenderer() = 0;
    };

    SLATECORE_API SlateRendererModule* GetSlateRendererModule();
}

