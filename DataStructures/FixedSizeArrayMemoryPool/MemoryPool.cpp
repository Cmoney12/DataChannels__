//
// Created by corey on 3/12/23.
//

#include "MemoryPool.h"

template <typename T>
MemoryPool<T>::MemoryPool(size_t chunk_size, size_t pool_size) :
        free_list_(nullptr), next_(nullptr), chunk_size_(chunk_size), pool_size_(pool_size) {
    size_t block_size = chunk_size_ + sizeof(Chunk);
    void* block = ::operator new(block_size * pool_size_);
    free_list_ = reinterpret_cast<Chunk*>(block);
    for (size_t i = 0; i < pool_size_ - 1; ++i) {
        free_list_[i].next = &free_list_[i + 1];
    }
    free_list_[pool_size_ - 1].next = nullptr;
    next_ = free_list_;
}

template <typename T>
MemoryPool<T>::~MemoryPool() {
    std::unique_lock<std::mutex> lock(mutex_);
    FreeAll();
    ::operator delete(free_list_);
}

template <typename T>
T* MemoryPool<T>::Allocate() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!next_) {
        return nullptr;
    }
    T* result = reinterpret_cast<T*>(next_ + 1);
    next_ = next_->next;
    return result;
}

template <typename T>
void MemoryPool<T>::Free(T* ptr) {
    std::unique_lock<std::mutex> lock(mutex_);
    Chunk* chunk = reinterpret_cast<Chunk*>(ptr) - 1;
    chunk->next = next_;
    next_ = chunk;
}

template <typename T>
void MemoryPool<T>::FreeAll() {
    for (Chunk* chunk = free_list_; chunk != nullptr; chunk = chunk->next) {
        chunk->next = next_;
        next_ = chunk;
    }
}