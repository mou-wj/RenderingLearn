#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "RHICommandList.h"
#include "RHICommandContex.h"
namespace RenderCore {

// 渲染命令结构体，包含名称和执行lambda
struct RENDERCORE_API RenderCommand
{
    std::string Name;
    std::function<void(RHI::RHIGraphicCommandList&)> Execute;

    RenderCommand(const std::string& InName, std::function<void(RHI::RHIGraphicCommandList&)> InFunc)
        : Name(InName), Execute(std::move(InFunc)) {}
};

RENDERCORE_API bool StartRenderThread();

RENDERCORE_API bool StopRenderThread();

RENDERCORE_API void EnqueueRenderCommand(const RenderCommand& cmd);

RENDERCORE_API void EnqueueRenderCommand(const std::string& cmdName, const std::function<void(RHI::RHIGraphicCommandList&)>& cmdFunc);

RENDERCORE_API void ExecuteSync(const RenderCommand& cmd);

RENDERCORE_API void ExecuteSync(const std::string& cmdName, const std::function<void(RHI::RHIGraphicCommandList&)>& cmdFunc);


// 简化版渲染线程管理类
class RENDERCORE_API RenderThread
{
public:
    RenderThread();
    ~RenderThread();

    void Start();
    void Stop();

    // 提交渲染命令到渲染线程
    void EnqueueCommand(const RenderCommand& cmd);

    void ExecuteSync(const RenderCommand& cmd);

    bool IsRunning() const { return bRunning.load(); }


private:
    void ThreadFunc();

    std::thread WorkerThread;
    std::atomic<bool> bRunning{ false };
    std::queue<RenderCommand> CommandQueue;
    std::mutex QueueMutex;
    std::condition_variable QueueCV;
    std::condition_variable CmdFinishCV;
    RHI::RHIQueue* ImmediateQueue = nullptr;
    RHI::RHIContextBase* ImmediateCommandContex = nullptr;
};



} // namespace RenderCore