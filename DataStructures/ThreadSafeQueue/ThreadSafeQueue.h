//
// Created by corey on 3/25/23.
//

#ifndef DATACHANNELS_THREADSAFEQUEUE_H
#define DATACHANNELS_THREADSAFEQUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>

template <class T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue(): queue(), mtx(), conditional_variable() {}

    ~ThreadSafeQueue() = default;

    // Add an element to the queue.
    void enqueue(const T& t) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(t);
        conditional_variable.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue() {
        std::unique_lock<std::mutex> lock(mtx);
        while(queue.empty())
        {
            // release lock as long as the wait and reaquire it afterwards.
            conditional_variable.wait(lock);
        }
        T val = queue.front();
        queue.pop();
        return val;
    }

private:
    std::queue<T> queue;
    mutable std::mutex mtx;
    std::condition_variable conditional_variable;
};

#endif //DATACHANNELS_THREADSAFEQUEUE_H
