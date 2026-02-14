#include "SlateRHIRenderer.h"
#include "RHIApi.h"
#include "RenderThread.h"
using namespace RHI;
using namespace RenderCore;
namespace SlateRHIRenderer {
	SlateRHIRenderer::SlateRHIRenderer() {

	}
	SlateRHIRenderer::~SlateRHIRenderer() {

	}
	void SlateRHIRenderer::Render(Slate::Window* window) {
		auto slateViewport = Viewports[window];
		auto rhiViewport = slateViewport.ViewportRHI;
		auto presentTexture =  GRHIApi->GetViewportBackBuffer(rhiViewport.get());
		//将场景的内容绘制到presentTexture上
		auto& windowWidget = window->GetWidgets();
		for (auto& widget : windowWidget) {
			auto widgetTeture = static_cast<RHI::RHITexture*>(widget.Viewport->GetViewportRenderTargetTexture());
			//绘制

		
		}
		
		//绘制overlay等UI内容到presentTexture上
		EnqueueRenderCommand("Present", [presentTexture, rhiViewport](RHI::RHICommandList& cmd) {

		

			});

	}

	void SlateRHIRenderer::CreateViewport(Slate::Window* window)
	{
		WindowViewportInfo viewportInfo;
		auto windowHandle = window->GetNativeHandle();
		auto framebufferSize = window->GetFramebufferSize();
		viewportInfo.ViewportRHI = GRHIApi->CreateViewport(windowHandle, framebufferSize.x, framebufferSize.y, ERHIFormat::R8G8B8A8_UNorm);

		Viewports[window] = viewportInfo;

	}



}