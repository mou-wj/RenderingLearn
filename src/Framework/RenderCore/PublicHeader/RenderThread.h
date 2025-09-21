#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace RenderCore {

// 渲染命令结构体，包含名称和执行lambda
struct RenderCommand
{
    std::string Name;
    std::function<void()> Execute;

    RenderCommand(const std::string& InName, std::function<void()> InFunc)
        : Name(InName), Execute(std::move(InFunc)) {}
};

// 简化版渲染线程管理类
class RenderThread
{
public:
    RenderThread();
    ~RenderThread();

    void Start();
    void Stop();

    // 提交渲染命令到渲染线程
    void EnqueueCommand(const RenderCommand& cmd);

    bool IsRunning() const { return bRunning.load(); }

private:
    void ThreadFunc();

    std::thread WorkerThread;
    std::atomic<bool> bRunning{ false };
    std::queue<RenderCommand> CommandQueue;
    std::mutex QueueMutex;
    std::condition_variable QueueCV;
};

} // namespace RenderCore