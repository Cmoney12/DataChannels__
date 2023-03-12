//
// Created by corey on 3/12/23.
//

#include "MemoryPool.h"

template <typename T>
MemoryPool<T>::MemoryPool(std::size_t chunkSize, std::size_t poolSize) : chunkSize_(chunkSize), poolSize_(poolSize) {
    static_assert(sizeof(T) >= chunkSize_, "Chunk size must be less than or equal to sizeof(T)");
    static_assert(poolSize_ % chunkSize_ == 0, "Pool size must be a multiple of chunk size");

    std::size_t numBlocks = poolSize_ / chunkSize_;
    blocks_ = reinterpret_cast<Block*>(::operator new(numBlocks * sizeof(Block)));

    for (std::size_t i = 0; i < numBlocks; ++i) {
        void* memory = &(blocks_[i].data);

        blocks_[i].next = freeList_;
        freeList_ = &blocks_[i];
    }
}

template <typename T>
T* MemoryPool<T>::allocate() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (!freeList_) {
        throw std::bad_alloc();
    }

    Block* block = freeList_;
    freeList_ = freeList_->next;

    T* memory = reinterpret_cast<T*>(&block->data);
    new (memory) T();

    return memory;
}

template <typename T>
void MemoryPool<T>::deallocate(T* memory) {
    std::unique_lock<std::mutex> lock(mutex_);

    Block* block = reinterpret_cast<Block*>(memory);
    block->next = freeList_;
    freeList_ = block;
}

template <typename T>
MemoryPool<T>::~MemoryPool() {
    std::unique_lock<std::mutex> lock(mutex_);

    std::size_t numBlocks = poolSize_ / chunkSize_;

    for (std::size_t i = 0; i < numBlocks; ++i) {
        blocks_[i].~Block();
    }

    ::operator delete(blocks_);
}