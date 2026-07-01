#include "SlateRHIRenderer.h"
#include "RHIApi.h"
#include "RenderThread.h"
#include "RenderResource.h"
#include "SlateViewport.h"
#include "Log.h"
#include "Common.h"
#include "Timer.h"
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
        
        CreateViewport(window);
        auto slateViewport = Viewports[window];
		auto rhiSwapchain = slateViewport.SwapchainRHI;
		auto slot = rhiSwapchain->AcquireNextSlot();
		auto backTexture = slot.Texture;
        if (backTexture == nullptr) { return; }
        LOG_INFO("render frame: %u",frameCount);
        LOG_INFO("duration time %f s", Core::Timer::GetGlobalInstance().GetDelta());
        frameCount++;
		//�����������ݻ��Ƶ�presentTexture��
		auto windowWidget = window->GetRootWidgets(); 
        std::vector<Slate::Widget*> widgets;
        widgets.push_back(windowWidget);
        auto computeTransitionContex = RHI::GRHIApi->GetQueue(EQueueType::Compute)->AcquireCommandContext();
        RHIComputeCommandList computeTransitionCmd(dynamic_cast<RHIComputeContex*>(computeTransitionContex));
        computeTransitionCmd.SetImmediate(true);
        
		auto graphicTransitionContex = RHI::GRHIApi->GetQueue(EQueueType::Graphics)->AcquireCommandContext();
        RHIGraphicCommandList graphicTransitionCmd(dynamic_cast<RHIGraphicContex*>(graphicTransitionContex));
        graphicTransitionCmd.SetImmediate(true);
        graphicTransitionCmd.Begin();
        std::vector<RHI::RHIWaitInfo> graphicWaitInfos;
        std::vector<RHI::RHIWaitInfo> computeWaitInfos;
        RHI::RHIWaitInfo graphicValidFinish;
        graphicValidFinish.QueueType = EQueueType::Graphics;
        graphicValidFinish.Value = 0;
        RHI::RHIWaitInfo computeValidFinish;
        computeValidFinish.QueueType = EQueueType::Compute;
        computeValidFinish.Value = 0;


        std::vector<RenderCore::RenderTexture*> renderTextures;
        bool hasCompute = false;
        for (auto& widget : widgets) {
            if (widget->IsA<Slate::SlateViewport>()) {
                auto slateW = widget->Cast<Slate::SlateViewport>();
                auto widgetTeture = static_cast<RenderCore::RenderTexture*>(slateW->GetViewportRenderTargetTexture());
                auto lastQueue = widgetTeture->GetTracker().GetLastAccessFence().QueueType;
				auto value = widgetTeture->GetTracker().GetLastAccessFence().Value;
                renderTextures.push_back(widgetTeture);
				if (lastQueue == EQueueType::Graphics) {
					TransitionResource(RHI::GRHIApi, graphicTransitionCmd, widgetTeture->GetRHI(), widgetTeture->GetTracker().GetSubresourceAccess(RHI::RHISubresourceRange{}), ERHIResourceAccess::TransferSrc, lastQueue, lastQueue);
					graphicValidFinish.Value = CORE_MAX(graphicValidFinish.Value, value);
                }
                else if (lastQueue == EQueueType::Compute) {
                    if (!hasCompute) {
                        computeTransitionCmd.Begin();
                    }
                    TransitionResource(RHI::GRHIApi, computeTransitionCmd, widgetTeture->GetRHI(), widgetTeture->GetTracker().GetSubresourceAccess(RHI::RHISubresourceRange{}), ERHIResourceAccess::TransferSrc, lastQueue, RHI::EQueueType::Graphics);
					hasCompute = true;
                    computeValidFinish.Value = CORE_MAX(computeValidFinish.Value, value);
                }

            }
        }
        //转换swapchain的access
        TransitionResource(RHI::GRHIApi, graphicTransitionCmd, backTexture, SwapChainTextureLastAccess[backTexture], ERHIResourceAccess::TransferDest, EQueueType::Graphics, EQueueType::Graphics);
        //blit texture

        for (auto& widget : widgets) {
            if (widget->IsA<Slate::SlateViewport>()) {
                auto slateW = widget->Cast<Slate::SlateViewport>();
                auto widgetTeture = static_cast<RenderCore::RenderTexture*>(slateW->GetViewportRenderTargetTexture());
                auto texDesc = widgetTeture->GetRHI()->GetDesc();
                RHI::RHIBlitTextureDesc blit{};
                blit.SrcRegion.Width = texDesc.Width;
                blit.SrcRegion.Height = texDesc.Height;
                blit.DstRegion.OffsetX = slateW->GetGeometry().X;
                blit.DstRegion.OffsetY = slateW->GetGeometry().Y;
                blit.DstRegion.Width = slateW->GetWidth();
                blit.DstRegion.Height = slateW->GetHeight();
                graphicTransitionCmd.BlitTexture(widgetTeture->GetRHI(), backTexture, blit);
            }
        }
        TransitionResource(RHI::GRHIApi, graphicTransitionCmd, backTexture, ERHIResourceAccess::TransferDest, ERHIResourceAccess::Present,EQueueType::Graphics,EQueueType::Graphics);
        if (slot.ReadySync)
        {
            graphicWaitInfos.push_back({ slot.ReadySync, EQueueType::Graphics, 0,RHI::ERHIPipelineStage::TopOfPipe });
        }

        if (hasCompute) {
			computeWaitInfos.push_back(computeValidFinish);
            
            computeTransitionCmd.End();
            computeTransitionCmd.ExecuteAll();
            auto transientComputeFence = RHI::GRHIApi->GetQueue(EQueueType::Compute)->ExecuteContext(computeTransitionContex);
            RHIWaitInfo waitInfoCompute;
            waitInfoCompute.QueueType = EQueueType::Compute;
			waitInfoCompute.Value = transientComputeFence.Value;
            graphicWaitInfos.push_back(waitInfoCompute);
        }
        else {
            RHI::GRHIApi->GetQueue(EQueueType::Compute)->ReleaseCommandContext(computeTransitionContex);
        }

		graphicTransitionCmd.End();
		
        
        auto transientGraphicFence = RHI::GRHIApi->GetQueue(EQueueType::Graphics)->ExecuteContext({ graphicTransitionContex }, graphicWaitInfos);

        RHI::RHIWaitInfo presentWait;
        presentWait.QueueType = transientGraphicFence.QueueType;
        presentWait.Value = transientGraphicFence.Value;

        RHI::GRHIApi->GetPresentExecutor()->Present(rhiSwapchain.get(), { presentWait });

        for (auto& renderTexture : renderTextures) {
            renderTexture->GetTracker().UpdateLastAccessFence(transientGraphicFence);
            renderTexture->GetTracker().UpdateSubresourceAccess(RHI::RHISubresourceRange{}, ERHIResourceAccess::TransferSrc);
        }

		
		}
		

	

	void SlateRHIRenderer::CreateViewport(Slate::Window* window)
	{
        if (Viewports.find(window) == Viewports.end()) {
            WindowViewportInfo viewportInfo;
            auto windowHandle = window->GetNativeHandle();
            auto framebufferSize = window->GetFramebufferSize();
            viewportInfo.SwapchainRHI = GRHIApi->CreateSwapchain(windowHandle, framebufferSize.x, framebufferSize.y, ERHIFormat::R8G8B8A8_UNorm);
            Viewports[window] = viewportInfo;
            Viewports[window].Width = framebufferSize.x;
            Viewports[window].Height = framebufferSize.y;
        }
        else {
            auto& viewportInfo = Viewports[window];
            auto framebufferSize = window->GetFramebufferSize();
            if (viewportInfo.Width != framebufferSize.x || viewportInfo.Height != framebufferSize.y) {
                viewportInfo.SwapchainRHI->Resize(framebufferSize.x, framebufferSize.y);
                viewportInfo.Width = framebufferSize.x;
                viewportInfo.Height = framebufferSize.y;
            }
        }

	}



}