//
// Created by corey on 3/12/23.
//

#ifndef DATACHANNELS_MEMORYPOOL_H
#define DATACHANNELS_MEMORYPOOL_H

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <memory>
#include <mutex>

template <typename T>
class MemoryPool {
public:
    MemoryPool(std::size_t chunkSize, std::size_t poolSize = std::numeric_limits<std::size_t>::max());

    T* allocate();

    void deallocate(T* memory);

    ~MemoryPool();

private:
    struct Block {
        Block* next;
        std::aligned_storage_t<sizeof(T), alignof(T)> data;
    };

    std::size_t chunkSize_;
    std::size_t poolSize_;
    Block* freeList_ = nullptr;
    std::unique_ptr<Block, void(*)(void*)> blockList_ {nullptr, [](void* ptr) {
        ::operator delete(ptr);
    }};
    std::mutex mutex_;
};

#endif //DATACHANNELS_MEMORYPOOL_H
