//
// Created by corey on 1/8/23.
//

#include "MemoryPool.h"

template <typename T>
T* MemoryPool<T>::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);  // Acquire the lock

    if (num_free_ == 0) {
        // The pool is empty, so we can't allocate any more memory
        return nullptr;
    }

    // Find the first available block of memory
    T* p = reinterpret_cast<T*>(&pool_[next_free_ * sizeof(T)]);
    next_free_ = *reinterpret_cast<int*>(p);  // Update next_free_ to the next available block
    num_free_--;
    return p;
}

template <typename T>
void MemoryPool<T>::release(T* p) {
    std::lock_guard<std::mutex> lock(mutex_);  // Acquire the lock

    *reinterpret_cast<int*>(p) = next_free_;  // Update the released block to point to the next available block
    next_free_ = (reinterpret_cast<std::uint8_t*>(p) - &pool_[0]) / sizeof(T);  // Update next_free_ to the released block
    num_free_++;
}

template <typename T>
void MemoryPool<T>::initialize(int pool_size) {
    std::lock_guard<std::mutex> lock(mutex_);  // Acquire the lock

    pool_.resize(pool_size * sizeof(T));

    // Initialize the linked list
    for (int i = 0; i < pool_size - 1; i++) {
        *reinterpret_cast<int*>(&pool_[i * sizeof(T)]) = i + 1;
    }
    *reinterpret_cast<int*>(&pool_[(pool_size - 1) * sizeof(T)]) = -1;  // Mark the last block as the end of the list

    next_free_ = 0;
    num_free_ = pool_size;
}
