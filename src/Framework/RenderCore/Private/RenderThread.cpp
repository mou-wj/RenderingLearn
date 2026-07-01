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
    // 1. ����һ�� promise �͹����� future
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    // 2. ��װԭʼ�����ִ��������� promise
    RenderCommand wrappedCmd = cmd;
    auto originalExecute = cmd.Execute;

    wrappedCmd.Execute = [originalExecute, &promise]() {
        if (originalExecute) {
            originalExecute();
        }
        // ��Ⱦ����ִ����ϣ�֪ͨ�ȴ��߳�
        promise.set_value();
        };

    // 3. ���ִ��
    EnqueueCommand(wrappedCmd);

    // 4. ������ǰ�̣߳�ֱ����Ⱦ�̵߳����� set_value()
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
            cmd.Execute();
        }
        CmdFinishCV.notify_one();
    }
}

RenderThread Instance;
RenderThread RHIInstance;

bool StartRenderThread() 
{
    Instance.Start();
    RHIInstance.Start();
	return true;
}

bool StopRenderThread() 
{
    RHIInstance.Stop();
	Instance.Stop();
	return true;
}

void EnqueueRenderCommand(const RenderCommand& cmd)
{
    Instance.EnqueueCommand(cmd);
}

void EnqueueRenderCommand(const std::string& cmdName, const std::function<void()>& cmdFunc)
{
	RenderCommand namedCmd(cmdName, cmdFunc);
	EnqueueRenderCommand(namedCmd);
}

RENDERCORE_API void EnqueueRHICommand(const std::string& cmdName, const std::function<void()>& cmdFunc)
{
    RenderCommand namedCmd(cmdName, cmdFunc);
    RHIInstance.EnqueueCommand(namedCmd);
}

RENDERCORE_API void ExecuteSync(const RenderCommand& cmd)
{
    Instance.ExecuteSync(cmd);
}

RENDERCORE_API void ExecuteSync(const std::string& cmdName, const std::function<void()>& cmdFunc)
{
    RenderCommand namedCmd(cmdName, cmdFunc);
    ExecuteSync(namedCmd);
}



} // namespace RenderCore