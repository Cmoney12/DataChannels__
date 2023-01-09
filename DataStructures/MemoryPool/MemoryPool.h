//
// Created by corey on 1/8/23.
//

#ifndef MEMORY_POOL_MEMORYPOOL_H
#define MEMORY_POOL_MEMORYPOOL_H

#include <cstdlib>
#include <vector>
#include <cstdint>
#include <mutex>

template <typename T>
class MemoryPool {
public:

    T* allocate();

    void release(T* p);

    void initialize(int pool_size);

private:
    std::vector<std::uint8_t> pool_;  // The memory pool
    int next_free_{};  // Index of the next available block of memory
    int num_free_{};  // Number of available blocks of memory
};

#endif //MEMORY_POOL_MEMORYPOOL_H
