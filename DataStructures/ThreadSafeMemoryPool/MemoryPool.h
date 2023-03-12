//
// Created by corey on 3/12/23.
//

#ifndef DATACHANNELS_MEMORYPOOL_H
#define DATACHANNELS_MEMORYPOOL_H

#include <cstddef>
#include <mutex>

template <typename T>
class MemoryPool {
public:
    MemoryPool(std::size_t chunkSize, std::size_t poolSize);
    T* allocate();
    void deallocate(T* memory);
    ~MemoryPool();

private:
    struct MemoryBlock {
        MemoryBlock* next;
    };

    const std::size_t chunkSize_;
    const std::size_t poolSize_;
    char* pool_;
    MemoryBlock* freeList_;
    MemoryBlock* allocatedList_;
    std::size_t blockIndex_;
    std::mutex mutex_;
};

#endif //DATACHANNELS_MEMORYPOOL_H
