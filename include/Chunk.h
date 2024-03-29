//
// Created by corey on 8/6/23.
//

#ifndef DATACHANNELS_CHUNK_H
#define DATACHANNELS_CHUNK_H

#include <vector>
#include <cstdint>
#include <memory>

template <typename T>
struct Chunk {
public:
    // Constructor with size
    explicit Chunk(std::size_t dataSize) : data(dataSize) {}

    // Constructor with move semantics
    Chunk(const T *dataToCopy, std::size_t dataLen) : data(dataToCopy, dataToCopy + dataLen) {}

    // Move constructor
    Chunk(Chunk &&other) noexcept : data(std::move(other.data)) {}

    // Move assignment operator
    Chunk &operator=(Chunk &&other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }

    [[nodiscard]] std::size_t Size() const { return data.size(); }
    [[nodiscard]] std::size_t Length() const { return Size(); }
    T *Data()  { return data.data(); }

private:
    std::vector<T> data;
};

template <typename T>
using ChunkPtr = std::shared_ptr<Chunk<T>>;

#endif //DATACHANNELS_CHUNK_H
