//
// Created by corey on 3/11/23.
//

#include "MemoryPool.h"

template <typename T, std::size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(std::size_t numBlocks) : numBlocks_{numBlocks} {
    // Allocate memory for the pool
    pool_ = std::aligned_alloc(alignof(T), BlockSize * numBlocks_);
    // Initialize the free list
    for (std::size_t i = 0; i < numBlocks_ - 1; ++i) {
        *reinterpret_cast<T**>(static_cast<std::uint8_t *>(pool_) + i * BlockSize) =
                reinterpret_cast<T*>(static_cast<std::uint8_t *>(pool_) + (i + 1) * BlockSize);
    }
    *reinterpret_cast<T**>(static_cast<std::uint8_t *>(pool_) + (numBlocks_ - 1) * BlockSize) = nullptr;
    freeList_ = reinterpret_cast<T*>(pool_);
}

template <typename T, std::size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() {
    std::free(pool_);
}

template <typename T, std::size_t BlockSize>
template<typename... Args>
T* MemoryPool<T, BlockSize>::allocate(Args&&... args) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!freeList_) {
        return nullptr;
    }
    T* block = freeList_;
    freeList_ = *reinterpret_cast<T**>(block);
    new (block) T(std::forward<Args>(args)...);
    return block;
}

template <typename T, std::size_t BlockSize>
void MemoryPool<T, BlockSize>::release(T* block) {
    std::lock_guard<std::mutex> lock(mutex_);
    block->~T();
    *reinterpret_cast<T**>(block) = freeList_;
    freeList_ = block;
}