#include "RenderThread.h"

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

void RenderThread::ThreadFunc() {
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
    }
}

} // namespace RenderCore