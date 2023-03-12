//
// Created by corey on 3/12/23.
//

#ifndef DATACHANNELS_MEMORYPOOL_H
#define DATACHANNELS_MEMORYPOOL_H

#include <mutex>
#include <cstdlib>

template <typename T>
class MemoryPool {
public:
    MemoryPool(std::size_t chunk_size, std::size_t pool_size);

    ~MemoryPool();

    T* Allocate();

    void Free(T* ptr);

    void FreeAll();

private:
    union Chunk {
        T data;
        Chunk* next;
    };

    Chunk* free_list_;
    Chunk* next_;
    std::size_t chunk_size_;
    std::size_t pool_size_;
    std::mutex mutex_;
};

#endif //DATACHANNELS_MEMORYPOOL_H
