#pragma once
#include "SlateRenderer.h"
#include "SlateViewport.h"
#include "RHIResource.h"
#include "RHICommandContex.h"
#include "Window.h"
#include <map>
namespace SlateRHIRenderer {


	struct ViewportDrawItem
	{
		Slate::SlateViewport* Viewport;
		int X, Y;
		int Width, Height;
	};

	struct WindowViewportInfo {
		RHI::RHISwapchainSP SwapchainRHI;
		int Width, Height;

	};

	class SlateRHIRenderer : public Slate::SlateRenderer {
	public:
		SlateRHIRenderer();
		~SlateRHIRenderer() override;
		virtual void Render(Slate::Window* window) override;
		void CreateViewport(Slate::Window* window) override;
	private:

		std::map<Slate::Window*, WindowViewportInfo> Viewports;
	};



}