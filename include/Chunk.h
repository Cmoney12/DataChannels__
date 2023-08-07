//
// Created by corey on 8/6/23.
//

#ifndef DATACHANNELS_CHUNK_H
#define DATACHANNELS_CHUNK_H

#include <vector>
#include <cstdint>

template <typename T>
class Chunk {
public:
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
    const T *Data() const { return data.data(); }

private:
    std::vector<T> data;
};

#endif //DATACHANNELS_CHUNK_H
