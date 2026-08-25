#include "OpenGLQueue.h"
#include "OpenGLContext.h"
#include "glad/gl.h"

namespace RHIOpenGL
{
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
        BackBufferTexture = std::make_shared<OpenGLTexture>(desc);
        BackBufferHandle = BackBufferTexture->GetHandle();
    }

    OpenGLSwapchain::~OpenGLSwapchain()
    {
        BackBufferTexture.reset();
    }

    RHI::RHISwapchain::RHISwapchainSlot OpenGLSwapchain::AcquireNextSlot()
    {
        RHISwapchainSlot slot{};
        slot.Texture = BackBufferTexture.get();
        return slot;
    }

    void OpenGLSwapchain::Resize(uint32_t width, uint32_t height)
    {
        Width = width;
        Height = height;

        if (BackBufferTexture)
        {
            RHI::RHITextureDesc desc = BackBufferTexture->GetDesc();
            desc.Width = width;
            desc.Height = height;
            desc.Depth = 1;
            BackBufferTexture = std::make_shared<OpenGLTexture>(desc);
            BackBufferHandle = BackBufferTexture->GetHandle();
        }
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

    RHI::RHIFence OpenGLQueue::ExecuteContext(RHI::RHIContextBase* context)
    {
        if (context)
        {
            context->End();
        }

        glFlush();
        return RHI::RHIFence{};
    }

    RHI::RHIFence OpenGLQueue::ExecuteContext(const std::vector<RHI::RHIContextBase*>& cmds, const std::vector<RHI::RHIWaitInfo>& waitInfos)
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
        return RHI::RHIFence{};
    }

    void OpenGLQueue::WaitFence(RHI::RHIFence fence)
    {
        (void)fence;
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

        auto* glSwapchain = dynamic_cast<OpenGLSwapchain*>(swapchain);
        if (glSwapchain)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, glSwapchain->GetWidth(), glSwapchain->GetHeight());
        }

        glFlush();
        glFinish();
    }
}
