#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include <vector>
#include <queue>


class ThreadPool {
public:
    void start();

    template<class Task, class ...Args>
    void enqueue_job(Task& task, Args&... args);

    void enqueue_job(std::function<void()>&& func);

    bool busy();

    void stop();

private:
    void thread_loop();

    bool should_terminate = false;
    std::mutex queue_mtx;
    std::condition_variable mtx_condition;
    std::vector<std::thread> thread_pool_;
    std::queue<std::function<void()>> task_queue;
};


