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
		auto rhiSwapchain = slateViewport.SwapchainRHI;
		auto slot = rhiSwapchain->AcquireNextSlot();
		auto presentTexture = slot.Texture;
		//�����������ݻ��Ƶ�presentTexture��
		auto& windowWidget = window->GetWidgets();
		for (auto& widget : windowWidget) {
			auto widgetTeture = static_cast<RHI::RHITexture*>(widget.Viewport->GetViewportRenderTargetTexture());
			//����

		
		}
		
		//����overlay��UI���ݵ�presentTexture��
		EnqueueRenderCommand("Present", [presentTexture, rhiSwapchain](RHI::RHIGraphicCommandList& cmd) {

		

			});

	}

	void SlateRHIRenderer::CreateViewport(Slate::Window* window)
	{
		WindowViewportInfo viewportInfo;
		auto windowHandle = window->GetNativeHandle();
		auto framebufferSize = window->GetFramebufferSize();
		viewportInfo.SwapchainRHI = GRHIApi->CreateSwapchain(windowHandle, framebufferSize.x, framebufferSize.y, ERHIFormat::R8G8B8A8_UNorm);

		Viewports[window] = viewportInfo;

	}



}