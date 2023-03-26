//
// Created by corey on 3/25/23.
//

#include "MemoryPool.h"

template<typename T>
MemoryPool<T>::MemoryPool(std::size_t chunkSize, std::size_t poolSize)
        : m_chunkSize{chunkSize}
        , m_poolSize{poolSize}
        , m_memory{std::make_unique<std::uint8_t[]>(m_chunkSize * m_poolSize)}
        , m_freeList{nullptr}
{}

template<typename T>
MemoryPool<T>::~MemoryPool() {
    clear();
}

template<typename T>
MemoryPool<T>::MemoryPool(MemoryPool&& other) noexcept
        : m_chunkSize{other.m_chunkSize}
        , m_poolSize{other.m_poolSize}
        , m_memory{std::move(other.m_memory)}
        , m_freeList{other.m_freeList}
{
    other.m_freeList = nullptr;
}

template<typename T>
MemoryPool<T>& MemoryPool<T>::operator=(MemoryPool&& other) noexcept {
    if (this != &other) {
        m_chunkSize = other.m_chunkSize;
        m_poolSize = other.m_poolSize;
        m_memory = std::move(other.m_memory);
        m_freeList = other.m_freeList;
        other.m_freeList = nullptr;
    }
    return *this;
}

template <typename T>
T* MemoryPool<T>::allocate() {
    std::unique_lock<std::mutex> lock{m_mutex};
    if (m_freeList == nullptr) {
        // Allocate additional memory
        std::uint8_t* newMemory = nullptr;
        try {
            newMemory = new std::uint8_t[m_chunkSize * (m_poolSize + 1)];
        } catch (...) {
            throw std::bad_alloc{};
        }
        // Initialize the new memory as a free list
        for (std::size_t i = 0; i < (m_poolSize + 1); ++i) {
            std::uint8_t* chunk = newMemory + (i * m_chunkSize);
            *reinterpret_cast<std::uint8_t**>(chunk) = m_freeList;
            m_freeList = chunk;
        }
        // Increase the pool size
        ++m_poolSize;
        // Release the old memory
        m_memory.reset(newMemory);
    }
    std::uint8_t* chunk = m_freeList;
    m_freeList = *reinterpret_cast<std::uint8_t**>(chunk);
    lock.unlock();
    return reinterpret_cast<T*>(chunk);
}

template <typename T>
void MemoryPool<T>::deallocate(T *ptr) {
    std::unique_lock<std::mutex> lock{m_mutex};
    std::uint8_t* chunk = reinterpret_cast<std::uint8_t*>(ptr);
    *reinterpret_cast<std::uint8_t**>(chunk) = m_freeList;
    m_freeList = chunk;
}

template <typename T>
void MemoryPool<T>::clear() {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_memory.reset();
    m_freeList = nullptr;
}