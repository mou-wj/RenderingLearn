#include "OpenGLQueue.h"
#include "OpenGLContext.h"
#include "glad/gl.h"
#include "OpenGLPlatformSurport.h"

namespace RHIOpenGL
{
    OpenGLSyncPoint::OpenGLSyncPoint() {

    }
    OpenGLSyncPoint::~OpenGLSyncPoint() {

    }
    // 获取当前 GPU 已经执行到的数值（用于 CPU 端的进度查询或 GC）
    uint64_t OpenGLSyncPoint::GetCurrentValue() {
        return 0;
    }

    // CPU 端阻塞等待直到达到某个值
    void OpenGLSyncPoint::Wait(uint64_t Value, uint64_t TimeoutNS) {

    }


    OpenGLSwapchain::OpenGLSwapchain(void* inWindowHandle, uint32_t width, uint32_t height, RHI::ERHIFormat format)
        : WindowHandle(inWindowHandle)
        , Width(width)
        , Height(height)
        , Format(format)
    {
        RHI::RHITextureDesc desc{};
        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = format;
        desc.Type = RHI::ERHITextureType::Texture2D;
        desc.Usage = RHI::ERHITextureCreateFlag::RenderTarget | RHI::ERHITextureCreateFlag::Presentable;
        for (int i = 0; i < 3; i++)
        {
            BackBufferTextures[i] = std::make_shared<OpenGLTexture>(desc);
        }
		PlatformContext = CreateOpenGLPlatformContext(WindowHandle,desc.Format);
        PlatformContext->Initialize();
    }

    OpenGLSwapchain::~OpenGLSwapchain()
    {
        BackBufferTextures.clear();
		if (PlatformContext)
		{
			delete PlatformContext;
			PlatformContext = nullptr;
		}
    }

    RHI::RHISwapchain::RHISwapchainSlot OpenGLSwapchain::AcquireNextSlot()
    {
        RHISwapchainSlot slot{};
        //slot.Texture = BackBufferTexture.get();
        return slot;
    }

    void OpenGLSwapchain::Resize(uint32_t width, uint32_t height)
    {
        Width = width;
        Height = height;

        for (auto& texture : BackBufferTextures) {
            RHI::RHITextureDesc desc = texture->GetDesc();
            desc.Width = width;
            desc.Height = height;
            desc.Depth = 1;
            texture = std::make_shared<OpenGLTexture>(desc);
        }
    }
    void OpenGLSwapchain::Present() {

    }
    OpenGLQueue::OpenGLQueue(RHI::EQueueType type)
        : QueueType(type)
    {
    }

    RHI::EQueueType OpenGLQueue::GetType() const
    {
        return QueueType;
    }

    RHI::RHIContextBase* OpenGLQueue::AcquireCommandContext()
    {
        if (QueueType == RHI::EQueueType::Compute)
        {
            return new OpenGLComputeContext(this);
        }

        return new OpenGLGraphicContext(this);
    }

    RHI::RHIContextBase* OpenGLQueue::ReleaseCommandContext(RHI::RHIContextBase* context)
    {
        delete context;
        return nullptr;
    }

    uint64_t OpenGLQueue::ExecuteContext(RHI::RHIContextBase* context)
    {
        if (context)
        {
            context->End();
        }

        glFlush();
        return 0;
    }

    uint64_t OpenGLQueue::ExecuteContext(const std::vector<RHI::RHIContextBase*>& cmds, const std::vector<RHI::RHIWaitInfo>& waitInfos)
    {
        (void)waitInfos;

        for (auto* cmd : cmds)
        {
            if (cmd)
            {
                cmd->End();
            }
        }

        glFlush();
        return 0;
    }

    void OpenGLQueue::WaitValue(uint64_t fenceValue)
    {
        glFinish();
    }

    void OpenGLQueue::WaitIdle()
    {
        glFinish();
    }

    uint64_t OpenGLQueue::GetCurrentTimelineValue()
    {
        return 0;
    }

    RHI::RHISyncPoint* OpenGLQueue::GetSyncPoint()
    {
        return nullptr;
    }

    OpenGLPresentExecutor::OpenGLPresentExecutor(OpenGLQueue* queue)
        : Queue(queue)
    {
    }

    void OpenGLPresentExecutor::Present(RHI::RHISwapchain* swapchain, const RHI::RHIWaitInfo& waitInfo)
    {
        (void)waitInfo;
        if (!swapchain)
        {
            return;
        }
        RHI::EQueueType queueType = RHI::EQueueType::Graphics;
		uint64_t waitValue = 0;
        if (waitInfo.SyncPoint) {
			queueType = waitInfo.SyncPoint->GetQueueType();
			waitValue = waitInfo.SyncPoint->GetCurrentValue();
        }
        else {
            queueType = waitInfo.QueueType;
            waitValue = waitInfo.Value;
        }
        RHI::RHIFence fence;
		fence.QueueType = queueType;
        fence.Value = waitValue;
		OpenGLQueueManager::GetInstance().GetQueue(queueType)->WaitValue(waitValue);

        auto* glSwapchain = dynamic_cast<OpenGLSwapchain*>(swapchain);
        if (glSwapchain)
        {
            glSwapchain->Present();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, glSwapchain->GetWidth(), glSwapchain->GetHeight());
        }

        glFlush();
        glFinish();
    }
}
