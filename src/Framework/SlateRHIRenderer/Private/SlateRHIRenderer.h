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
		SlateCore::SlateViewport* Viewport;
		int X, Y;
		int Width, Height;
	};

	struct WindowViewportInfo {
		RHI::RHISwapchainSP SwapchainRHI;
		int Width, Height;

	};

	class SlateRHIRenderer : public SlateCore::SlateRenderer {
	public:
		SlateRHIRenderer();
		~SlateRHIRenderer() override;
		virtual void Render(SlateCore::Window* window) override;
		void CreateViewport(SlateCore::Window* window) override;
	private:

		std::map<SlateCore::Window*, WindowViewportInfo> Viewports;
	};



}