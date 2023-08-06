//
// Created by corey on 8/6/23.
//

#ifndef DATACHANNELS_DATAPACKET_H
#define DATACHANNELS_DATAPACKET_H

#include <memory>
#include <vector>
#include <cstdint>

class DataPacket {
public:
    DataPacket(size_t size) : buffer(size), size(size) {}

    [[nodiscard]] std::size_t getSize() const {
        return size;
    }

    std::uint8_t* getData() {
        return buffer.data();
    }

private:
    std::vector<std::uint8_t> buffer;
    size_t size;
};

using DataPacketPtr = std::shared_ptr<DataPacket>;

#endif //DATACHANNELS_DATAPACKET_H
