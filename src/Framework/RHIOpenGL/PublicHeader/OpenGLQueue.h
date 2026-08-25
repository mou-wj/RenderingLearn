#pragma once

#include "RHICommandContex.h"
#include "OpenGLResource.h"
#include <memory>

namespace RHIOpenGL
{
    class OpenGLSwapchain : public RHI::RHISwapchain
    {
    public:
        OpenGLSwapchain(void* inWindowHandle, uint32_t width, uint32_t height, RHI::ERHIFormat format);
        ~OpenGLSwapchain() override;

        RHISwapchainSlot AcquireNextSlot() override;
        void Resize(uint32_t width, uint32_t height) override;

        uint32_t GetWidth() const { return Width; }
        uint32_t GetHeight() const { return Height; }

    private:
        void* WindowHandle = nullptr;
        uint32_t Width = 0;
        uint32_t Height = 0;
        RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
        GLuint BackBufferHandle = 0;
        std::shared_ptr<OpenGLTexture> BackBufferTexture;
    };
    using OpenGLSwapchainSP = std::shared_ptr<OpenGLSwapchain>;

    class OpenGLQueue : public RHI::RHIQueue
    {
    public:
        explicit OpenGLQueue(RHI::EQueueType type = RHI::EQueueType::Graphics);
        ~OpenGLQueue() override = default;

        RHI::EQueueType GetType() const override;
        RHI::RHIContextBase* AcquireCommandContext() override;
        RHI::RHIContextBase* ReleaseCommandContext(RHI::RHIContextBase* context) override;
        RHI::RHIFence ExecuteContext(RHI::RHIContextBase* context) override;
        RHI::RHIFence ExecuteContext(const std::vector<RHI::RHIContextBase*>& cmds, const std::vector<RHI::RHIWaitInfo>& waitInfos) override;
        void WaitFence(RHI::RHIFence fence) override;
        void WaitIdle() override;
        uint64_t GetCurrentTimelineValue() override;
        RHI::RHISyncPoint* GetSyncPoint() override;

    private:
        RHI::EQueueType QueueType = RHI::EQueueType::Graphics;
    };

    class OpenGLPresentExecutor : public RHI::RHIPresentExecutor
    {
    public:
        explicit OpenGLPresentExecutor(OpenGLQueue* queue = nullptr);
        ~OpenGLPresentExecutor() override = default;

        void Present(RHI::RHISwapchain* swapchain, const RHI::RHIWaitInfo& waitInfo) override;

    private:
        OpenGLQueue* Queue = nullptr;
    };
}
