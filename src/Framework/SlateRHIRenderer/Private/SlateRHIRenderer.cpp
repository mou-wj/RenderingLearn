#include "SlateRHIRenderer.h"
#include "RHIApi.h"
#include "RenderThread.h"
#include "RenderResource.h"
#include "SlateViewport.h"
#include "Log.h"
#include "RHIApi.h"
using namespace RHI;
using namespace RenderCore;
namespace SlateRHIRenderer {
    std::unordered_map<RHI::RHITexture*, ERHIResourceAccess> SwapChainTextureLastAccess;

	SlateRHIRenderer::SlateRHIRenderer() {

	}
	SlateRHIRenderer::~SlateRHIRenderer() {

	}
    uint64_t frameCount = 0;
	void SlateRHIRenderer::Render(Slate::Window* window) {
		auto slateViewport = Viewports[window];
		auto rhiSwapchain = slateViewport.SwapchainRHI;
		auto slot = rhiSwapchain->AcquireNextSlot();
		auto backTexture = slot.Texture;
        if (backTexture == nullptr) { return; }
        LOG_INFO("render frame: %u",frameCount);
        frameCount++;
		//�����������ݻ��Ƶ�presentTexture��
		auto windowWidget = window->GetRootWidgets();
        auto* computeQueue = RHI::GRHIApi->GetQueue(EQueueType::Compute);
        auto* ctx = computeQueue->AcquireCommandContext();
        RHIComputeCommandList cmd(dynamic_cast<RHIComputeContex*>(ctx));
        cmd.SetImmediate(true);
        cmd.Begin();
        auto api = RHI::GRHIApi;
        TransitionResource(api, cmd, backTexture, SwapChainTextureLastAccess[backTexture], ERHIResourceAccess::TransferDest);
        std::vector<RHI::RHIWaitInfo> waitInfos;
        std::vector<RenderCore::RenderTexture*> renderTextures;
        std::vector<Slate::Widget*> widgets;
        widgets.push_back(windowWidget);
        for (auto& widget : widgets) {
			if (widget->IsA<Slate::SlateViewport>()) {
				auto slateW = widget->Cast<Slate::SlateViewport>();
				auto widgetTeture = static_cast<RenderCore::RenderTexture*>(slateW->GetViewportRenderTargetTexture());
				renderTextures.push_back(widgetTeture);
                TransitionResource(api, cmd, widgetTeture->GetRHI(), widgetTeture->GetTracker().GetSubresourceAccess(RHI::RHISubresourceRange{}), ERHIResourceAccess::TransferSrc);
                auto texDesc = widgetTeture->GetRHI()->GetDesc();
                RHI::RHIWaitInfo renderTargetFinish;
                renderTargetFinish.QueueType = widgetTeture->GetTracker().GetLastAccessFence().QueueType;
                renderTargetFinish.Value = widgetTeture->GetTracker().GetLastAccessFence().Value;
                waitInfos.push_back(renderTargetFinish);


                RHI::RHIBlitTextureDesc blit{};
                blit.SrcRegion.Width = texDesc.Width;
                blit.SrcRegion.Height = texDesc.Height;
				blit.DstRegion.OffsetX = slateW->GetGeometry().X;
                blit.DstRegion.OffsetY = slateW->GetGeometry().Y;
                blit.DstRegion.Width = slateW->GetWidth();
                blit.DstRegion.Height = slateW->GetHeight();
                cmd.BlitTexture(widgetTeture->GetRHI(), backTexture, blit);

                


            }
            TransitionResource(api, cmd, backTexture, ERHIResourceAccess::TransferDest, ERHIResourceAccess::Present);
            cmd.End();
            SwapChainTextureLastAccess[backTexture] = ERHIResourceAccess::Present;


            if (slot.ReadySync)
            {
                waitInfos.push_back({ slot.ReadySync, EQueueType::Graphics, 0,RHI::ERHIPipelineStage::ColorAttachmentOutput });
            }
            auto fence = computeQueue->ExecuteContext({ ctx }, waitInfos);
            RHI::RHIWaitInfo presentWait;
            presentWait.QueueType = fence.QueueType;
            presentWait.Value = fence.Value;
			api->GetQueue(EQueueType::Compute)->WaitFence(fence);

            api->GetPresentExecutor()->Present(rhiSwapchain.get(), { presentWait });
            // 更新 tracker（关键！）
            for (auto& renderTexture : renderTextures) {
                renderTexture->GetTracker().UpdateLastAccessFence(fence);
                renderTexture->GetTracker().UpdateSubresourceAccess(RHI::RHISubresourceRange{}, ERHIResourceAccess::TransferSrc);
            }

			}
			//auto widgetTeture = static_cast<RHI::RHITexture*>(widget.Viewport->GetViewportRenderTargetTexture());
			//����

		
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