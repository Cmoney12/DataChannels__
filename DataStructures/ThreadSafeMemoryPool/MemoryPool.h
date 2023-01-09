//
// Created by corey on 1/8/23.
//

#ifndef MEMORY_POOL_MEMORYPOOL_H
#define MEMORY_POOL_MEMORYPOOL_H

#include <cstdlib>
#include <vector>
#include <mutex>

template <typename T>
class MemoryPool {
public:
    // Allocates a block of memory from the pool
    T* allocate();

    // Releases a block of memory back to the pool
    void release(T* p);

    // Initializes the memory pool
    void initialize(int pool_size);

private:
    std::vector<std::uint8_t> pool_;  // The memory pool
    int next_free_{};  // Index of the next available block of memory
    int num_free_{};  // Number of available blocks of memory
    std::mutex mutex_;  // Mutex for synchronization
};

#endif //MEMORY_POOL_MEMORYPOOL_H
