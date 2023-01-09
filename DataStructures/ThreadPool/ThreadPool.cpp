#include "ThreadPool.h"

void ThreadPool::start() {
    const uint32_t num_threads = std::thread::hardware_concurrency(); // Max # of threads the system supports
    thread_pool_.resize(num_threads);
    for (auto i = 0; i < num_threads; i++) {
        thread_pool_.at(i) = std::thread([this] { thread_loop(); });
    }
}

template<class Task, class ...Args>
void ThreadPool::enqueue_job(Task &task, Args& ... args) {
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        task_queue.push(std::bind(std::forward<Task>(task), std::forward<Args>(args)...));
    }
    mtx_condition.notify_one();
}

void ThreadPool::enqueue_job(std::function<void()> &&func) {
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        task_queue.push(func);
    }
    mtx_condition.notify_one();
}


void ThreadPool::thread_loop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(queue_mtx);
            mtx_condition.wait(lock, [this] {
                return !task_queue.empty() || should_terminate;
            });
            if (should_terminate) {
                return;
            }
            job = task_queue.front();
            task_queue.pop();
        }
        job();
    }
}

bool ThreadPool::busy() {
    bool pool_busy;
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        pool_busy = task_queue.empty();
    }
    return pool_busy;
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        should_terminate = true;
    }
    mtx_condition.notify_all();
    for (auto &i: thread_pool_) {
        i.join();
    }
    thread_pool_.clear();
}


