#pragma once
#include "Module.h"
#include "SlateViewport.h"
#include "Window.h"
namespace Slate {

    class SLATE_API SlateRenderer {
    public:
        virtual ~SlateRenderer() = default;
        virtual void Render(Window* window) = 0;
        virtual void CreateViewport(Window* window) = 0;

    };
	using SlateRendererSP = std::shared_ptr<SlateRenderer>;

    class SLATE_API SlateRendererModule : public Core::Module {
    public:
        virtual SlateRenderer* CreateSlateRenderer() = 0;
    };

    SLATE_API SlateRendererModule* GetSlateRendererModule();
}

