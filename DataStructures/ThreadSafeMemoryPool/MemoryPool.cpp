//
// Created by corey on 3/12/23.
//

#include "MemoryPool.h"

#include <stdexcept>

template <typename T>
MemoryPool<T>::MemoryPool(std::size_t chunkSize, std::size_t poolSize) :
        chunkSize_(chunkSize), poolSize_(poolSize), freeList_(nullptr), allocatedList_(nullptr), blockIndex_(0)
{
    pool_ = new char[poolSize_];

    std::unique_lock<std::mutex> lock(mutex_);

    for (std::size_t i = 0; i < poolSize_; i += chunkSize_) {
        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(&pool_[i]);
        block->next = freeList_;
        freeList_ = block;
    }
}

template <typename T>
T* MemoryPool<T>::allocate() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (!freeList_) {
        throw std::bad_alloc();
    }

    T* memory = reinterpret_cast<T*>(freeList_);
    freeList_ = freeList_->next;

    MemoryBlock* block = reinterpret_cast<MemoryBlock*>(memory);
    block->next = allocatedList_;
    allocatedList_ = block;

    return memory;
}

template <typename T>
void MemoryPool<T>::deallocate(T* memory) {
    std::unique_lock<std::mutex> lock(mutex_);

    MemoryBlock* block = reinterpret_cast<MemoryBlock*>(memory);
    block->next = freeList_;
    freeList_ = block;

    if (allocatedList_ == block) {
        allocatedList_ = block->next;
    }
    else {
        MemoryBlock* prev = allocatedList_;
        while (prev->next != block) {
            prev = prev->next;
        }
        prev->next = block->next;
    }
}

template <typename T>
MemoryPool<T>::~MemoryPool() {
    std::unique_lock<std::mutex> lock(mutex_);

    delete[] pool_;

    MemoryBlock* block = allocatedList_;
    while (block) {
        MemoryBlock* nextBlock = block->next;
        delete[] reinterpret_cast<char*>(block);
        block = nextBlock;
    }
}
