#include "TaskPool.h"

#include <cassert>
#include <chrono>
#include <utility>

namespace Core {

TaskPool::TaskPool(size_t threads) {
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 1;
    }

    bShutdown.store(false);
    Workers.reserve(threads);
    for (size_t i = 0; i < threads; ++i) {
        Workers.emplace_back(&TaskPool::WorkerLoop, this);
    }
}

TaskPool::~TaskPool() {
    // Initiate shutdown
    {
        std::lock_guard<std::mutex> lock(QueueMutex);
        bShutdown.store(true);
    }
    QueueCV.notify_all();

    // Join all workers
    for (auto& t : Workers) {
        if (t.joinable()) t.join();
    }

    // In case any pending tasks remained, set exceptions / values so futures won't hang.
    // However the design expects workers to finish executing tasks prior to shutdown.
}

TaskHandle TaskPool::AddTask(std::function<void()> task) {
    if (!task) {
        return TaskHandle{};
    }

    auto promisePtr = std::make_shared<std::promise<void>>();
    std::shared_future<void> sf = promisePtr->get_future().share();

    uint64_t id = NextId.fetch_add(1, std::memory_order_relaxed);

    TaskItem item;
    item.Id = id;
    item.Func = std::move(task);
    item.PromisePtr = promisePtr;

    {
        std::lock_guard<std::mutex> lock(QueueMutex);
        Queue.push(std::move(item));
        PendingTasks.fetch_add(1, std::memory_order_relaxed);
    }

    QueueCV.notify_one();

    return TaskHandle{ id, sf };
}

std::vector<TaskHandle> TaskPool::AddTasks(const std::vector<std::function<void()>>& tasks) {
    std::vector<TaskHandle> handles;
    handles.reserve(tasks.size());

    // Reserve: we will hold lock briefly to push all tasks
    std::lock_guard<std::mutex> lock(QueueMutex);
    for (const auto& task : tasks) {
        if (!task) {
            handles.push_back(TaskHandle{ 0, std::shared_future<void>() });
            continue;
        }
        auto promisePtr = std::make_shared<std::promise<void>>();
        std::shared_future<void> sf = promisePtr->get_future().share();

        uint64_t id = NextId.fetch_add(1, std::memory_order_relaxed);
        TaskItem item;
        item.Id = id;
        item.Func = task;
        item.PromisePtr = promisePtr;

        Queue.push(std::move(item));
        handles.push_back(TaskHandle{ id, sf });
        PendingTasks.fetch_add(1, std::memory_order_relaxed);
    }
    QueueCV.notify_all();
    return handles;
}

void TaskPool::Wait(const TaskHandle& handle) const {
    if (handle.IsValid()) {
        handle.Future.wait();
    }
}

void TaskPool::WaitAll(const std::vector<TaskHandle>& handles) const {
    for (const auto& h : handles) {
        if (h.IsValid()) h.Future.wait();
    }
}

void TaskPool::WaitAll() const {
    std::unique_lock<std::mutex> lock(QueueMutex);
    PendingCV.wait(lock, [this]() {
        return PendingTasks.load(std::memory_order_relaxed) == 0;
    });
}

bool TaskPool::IsCompleted(const TaskHandle& handle) const {
    if (!handle.IsValid()) return true;
    return handle.IsCompleted();
}

void TaskPool::WorkerLoop() {
    while (true) {
        TaskItem taskItem;
        {
            std::unique_lock<std::mutex> lock(QueueMutex);
            QueueCV.wait(lock, [this]() {
                return bShutdown.load() || !Queue.empty();
            });

            if (bShutdown.load() && Queue.empty()) {
                return; // clean exit
            }

            if (Queue.empty()) {
                continue;
            }

            taskItem = std::move(Queue.front());
            Queue.pop();
        }

        // Execute outside lock
        try {
            if (taskItem.Func) taskItem.Func();
            if (taskItem.PromisePtr) taskItem.PromisePtr->set_value();
        } catch (...) {
            if (taskItem.PromisePtr) {
                try {
                    taskItem.PromisePtr->set_exception(std::current_exception());
                } catch (...) {
                    // set_exception might throw if already set; ignore
                }
            }
        }

        // Decrement pending count and notify any waiting WaitAll
        PendingTasks.fetch_sub(1, std::memory_order_relaxed);
        PendingCV.notify_all();
    }
}

} // namespace Common