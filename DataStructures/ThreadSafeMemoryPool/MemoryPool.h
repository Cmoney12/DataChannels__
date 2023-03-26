//
// Created by corey on 3/25/23.
//

#ifndef DATACHANNELS_MEMORYPOOL_H
#define DATACHANNELS_MEMORYPOOL_H

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>

template<typename T>
class MemoryPool {
public:
    MemoryPool(std::size_t chunkSize, std::size_t poolSize);
    ~MemoryPool();
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&& other) noexcept;
    MemoryPool& operator=(MemoryPool&& other) noexcept;
    T* allocate();
    void deallocate(T* ptr);
    void clear();

private:
    std::size_t m_chunkSize;
    std::size_t m_poolSize;
    std::unique_ptr<std::uint8_t[]> m_memory;
    std::uint8_t* m_freeList;
    std::mutex m_mutex;
};

#endif //DATACHANNELS_MEMORYPOOL_H
