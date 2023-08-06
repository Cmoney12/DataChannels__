//
// Created by corey on 8/6/23.
//

#ifndef DATACHANNELS_CIRCULARBUFFER_H
#define DATACHANNELS_CIRCULARBUFFER_H

#include <vector>
#include <mutex>

class CircularBuffer {
public:
    CircularBuffer(size_t bufferSize) : buffer(bufferSize), writeIndex(0), readIndex(0) {}

    size_t getSize() const {
        return buffer.size();
    }

    void write(const uint8_t* data, size_t dataSize) {
        std::lock_guard<std::mutex> lock(mutex);
        for (size_t i = 0; i < dataSize; ++i) {
            buffer[writeIndex] = data[i];
            writeIndex = (writeIndex + 1) % buffer.size();
        }
    }

    void read(uint8_t* data, size_t dataSize) {
        std::lock_guard<std::mutex> lock(mutex);
        for (size_t i = 0; i < dataSize; ++i) {
            data[i] = buffer[readIndex];
            readIndex = (readIndex + 1) % buffer.size();
        }
    }

private:
    std::vector<uint8_t> buffer;
    size_t writeIndex;
    size_t readIndex;
    std::mutex mutex;
};

#endif //DATACHANNELS_CIRCULARBUFFER_H
