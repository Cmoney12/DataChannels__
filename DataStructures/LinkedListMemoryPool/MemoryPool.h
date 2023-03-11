//
// Created by corey on 3/11/23.
//

#ifndef DATACHANNELS_MEMORYPOOL_H
#define DATACHANNELS_MEMORYPOOL_H

#include <cstddef>
#include <mutex>
#include <type_traits>
#include <new>

template <typename T, std::size_t BlockSize>
class MemoryPool {
public:
    // Constructor that initializes the memory pool with a given number of blocks
    explicit MemoryPool(std::size_t numBlocks);

    // Destructor that frees the memory pool
    ~MemoryPool();

    // Allocate a block of memory from the pool
    template<typename... Args>
    T* allocate(Args&&... args);

    // Release a block of memory back to the pool
    void release(T* block);

private:
    std::size_t numBlocks_; // Number of blocks in the pool
    void* pool_; // Pointer to the memory pool
    std::mutex mutex_; // Mutex for thread-safe allocation
    T* freeList_; // Head of the free list
};


#endif //DATACHANNELS_MEMORYPOOL_H
