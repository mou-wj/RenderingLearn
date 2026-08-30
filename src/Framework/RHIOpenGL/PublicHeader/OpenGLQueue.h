#pragma once

#include "RHICommandContex.h"
#include "OpenGLResource.h"
#include "Singleton.hpp"
#include <memory>
#include <vector>

namespace RHIOpenGL
{
    class OpenGLPlatformContext;
    class OpenGLSyncPoint : public RHI::RHISyncPoint
    {
	public:
		OpenGLSyncPoint();
		~OpenGLSyncPoint() override;
        // 获取当前 GPU 已经执行到的数值（用于 CPU 端的进度查询或 GC）
        uint64_t GetCurrentValue() override;

        // CPU 端阻塞等待直到达到某个值
        void Wait(uint64_t Value, uint64_t TimeoutNS = UINT64_MAX) override;
    };

    class OpenGLSwapchain : public RHI::RHISwapchain
    {
    public:
        OpenGLSwapchain(void* inWindowHandle, uint32_t width, uint32_t height, RHI::ERHIFormat format);
        ~OpenGLSwapchain() override;

        RHISwapchainSlot AcquireNextSlot() override;
        void Resize(uint32_t width, uint32_t height) override;

        uint32_t GetWidth() const { return Width; }
        uint32_t GetHeight() const { return Height; }

		void Present();

    private:
        void* WindowHandle = nullptr;
        uint32_t Width = 0;
        uint32_t Height = 0;
        RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
		uint32_t CurrentBackBufferIndex = 0;
        std::vector<RHI::RHIFence> BackBufferFences;
		std::vector<std::shared_ptr<OpenGLTexture>> BackBufferTextures;
		OpenGLPlatformContext* PlatformContext = nullptr;
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
        uint64_t ExecuteContext(RHI::RHIContextBase* context) override;
        uint64_t ExecuteContext(const std::vector<RHI::RHIContextBase*>& cmds, const std::vector<RHI::RHIWaitInfo>& waitInfos) override;
        void WaitValue(uint64_t fenceValue) override;
        void WaitIdle() override;
        uint64_t GetCurrentTimelineValue() override;
        RHI::RHISyncPoint* GetSyncPoint() override;

    private:
        RHI::EQueueType QueueType = RHI::EQueueType::Graphics;
    };

    class OpenGLQueueManager : public Singleton<OpenGLQueueManager> {
		friend class Singleton<OpenGLQueueManager>;
    public:
		OpenGLQueue* GetQueue(RHI::EQueueType type) {
			if (type == RHI::EQueueType::Graphics) {
                if (!GraphicsQueue) {
                    GraphicsQueue = new OpenGLQueue(RHI::EQueueType::Graphics);
                }
                return GraphicsQueue;
			}
			else if (type == RHI::EQueueType::Compute) {
				if (!ComputeQueue) {
					ComputeQueue = new OpenGLQueue(RHI::EQueueType::Compute);
				}
				return ComputeQueue;
			}
            if (!GraphicsQueue) {
                GraphicsQueue = new OpenGLQueue(RHI::EQueueType::Graphics);
            }
            return GraphicsQueue;
		}
        void Destroy() {
			if (GraphicsQueue) {
				delete GraphicsQueue;
				GraphicsQueue = nullptr;
			}
			if (ComputeQueue) {
				delete ComputeQueue;
				ComputeQueue = nullptr;
			}
		}
    private:
		OpenGLQueue* GraphicsQueue = nullptr;
        OpenGLQueue* ComputeQueue = nullptr;
		OpenGLQueueManager() = default;
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
