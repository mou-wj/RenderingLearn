#pragma once
#include "Module.h"
#include "SlateViewport.h"
#include "SlateWidget.h"
namespace SlateCore {

    class SLATECORE_API SlateRenderer {
    public:
        virtual ~SlateRenderer() = default;
        virtual void Render(SlateWidget* slateWidget) = 0;
        virtual void CreateViewport(SlateWidget* slateWidget) = 0;

    };
	using SlateRendererSP = std::shared_ptr<SlateRenderer>;

    class SLATECORE_API SlateRendererModule : public Core::Module {
    public:
        virtual SlateRenderer* CreateSlateRenderer() = 0;
    };

    SLATECORE_API SlateRendererModule* GetSlateRendererModule();
}

