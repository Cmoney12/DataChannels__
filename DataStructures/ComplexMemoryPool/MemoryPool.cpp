//
// Created by corey on 3/12/23.
//

#include "MemoryPool.h"

template <typename T>
MemoryPool<T>::MemoryPool(std::size_t chunkSize, std::size_t poolSize)
        : chunkSize_(chunkSize), poolSize_(poolSize) {
    static_assert(sizeof(T) >= chunkSize_, "Chunk size must be less than or equal to sizeof(T)");
    static_assert(poolSize_ % chunkSize_ == 0, "Pool size must be a multiple of chunk size");

    std::size_t numBlocks = poolSize_ / chunkSize_;
    Block* prevBlock = nullptr;

    for (std::size_t i = 0; i < numBlocks; ++i) {
        Block* block = reinterpret_cast<Block*>(::operator new(sizeof(Block)));
        block->next = prevBlock;
        prevBlock = block;

        void* memory = &(block->data);

        for (std::size_t j = 0; j < chunkSize_ / sizeof(T); ++j) {
            T* ptr = reinterpret_cast<T*>(memory);
            new (ptr) T();
            memory = static_cast<char*>(memory) + sizeof(T);
        }
    }

    blockList_.reset(prevBlock);
    freeList_ = prevBlock;
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

    Block* block = blockList_.release();

    while (block) {
        Block* nextBlock = block->next;

        void* memory = &(block->data);

        for (std::size_t i = 0; i < chunkSize_ / sizeof(T); ++i) {
            T* ptr = reinterpret_cast<T*>(memory);
            ptr->~T();
            memory = static_cast<char*>(memory) + sizeof(T);
        }

        ::operator delete(block);

        block = nextBlock;
    }
}
