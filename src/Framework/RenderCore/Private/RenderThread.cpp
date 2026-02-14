#include "RenderThread.h"
#include "RHIApi.h"
#include <future>
#include "ThreadInfo.h"
using namespace Core;
namespace RenderCore {

RenderThread::RenderThread() : bRunning(false) {}

RenderThread::~RenderThread() {
    Stop();
}

void RenderThread::Start() {
    if (bRunning.load()) return;
    bRunning.store(true);
    WorkerThread = std::thread(&RenderThread::ThreadFunc, this);
    //创建ImmediateCommandContex
    ImmediateCommandContex = RHI::GRHIApi->GetDefualtCommandContex();
}

void RenderThread::Stop() {
    if (!bRunning.load()) return;
    {
        std::lock_guard<std::mutex> lock(QueueMutex);
        bRunning.store(false);
        QueueCV.notify_all();
    }
    if (WorkerThread.joinable()) {
        WorkerThread.join();
    }
}

void RenderThread::EnqueueCommand(const RenderCommand& cmd) {
    {
        std::lock_guard<std::mutex> lock(QueueMutex);
        CommandQueue.push(cmd);
    }
    QueueCV.notify_one();
}

void RenderThread::ExecuteSync(const RenderCommand& cmd) {
    // 1. 创建一个 promise 和关联的 future
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    // 2. 包装原始命令，在执行完后设置 promise
    RenderCommand wrappedCmd = cmd;
    auto originalExecute = cmd.Execute;

    wrappedCmd.Execute = [originalExecute, &promise](auto& commandList) {
        if (originalExecute) {
            originalExecute(commandList);
        }
        // 渲染任务执行完毕，通知等待线程
        promise.set_value();
        };

    // 3. 入队执行
    EnqueueCommand(wrappedCmd);

    // 4. 阻塞当前线程，直到渲染线程调用了 set_value()
    future.wait();
}

void RenderThread::ThreadFunc() {
    InitRenderThreadId();
    while (bRunning.load() || !CommandQueue.empty()) {
        RenderCommand cmd("", nullptr);
        {
            std::unique_lock<std::mutex> lock(QueueMutex);
            QueueCV.wait(lock, [this] { return !CommandQueue.empty() || !bRunning.load(); });
            if (!CommandQueue.empty()) {
                cmd = std::move(CommandQueue.front());
                CommandQueue.pop();
            } else {
                continue;
            }
        }
        if (cmd.Execute) {
            cmd.Execute(ImmediateCommandContex->GetCommandList());
            ImmediateCommandContex->GetCommandList().ExecuteAll();
            ImmediateCommandContex->GetCommandList().Clear();
        }
        CmdFinishCV.notify_one();
    }
}

RenderThread Instance;

bool StartRenderThread() 
{
	Instance.Start();
	return true;
}

bool StopRenderThread() 
{
	Instance.Stop();
	return true;
}

void EnqueueRenderCommand(const RenderCommand& cmd)
{
    Instance.EnqueueCommand(cmd);
}

void EnqueueRenderCommand(const std::string& cmdName, const std::function<void(RHI::RHICommandList&)>& cmdFunc)
{
	RenderCommand namedCmd(cmdName, cmdFunc);
	EnqueueRenderCommand(namedCmd);
}

RENDERCORE_API void ExecuteSync(const RenderCommand& cmd)
{
    Instance.ExecuteSync(cmd);
}

RENDERCORE_API void ExecuteSync(const std::string& cmdName, const std::function<void(RHI::RHICommandList&)>& cmdFunc)
{
    RenderCommand namedCmd(cmdName, cmdFunc);
    ExecuteSync(namedCmd);
}



} // namespace RenderCore