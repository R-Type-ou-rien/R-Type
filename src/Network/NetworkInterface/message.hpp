#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#define MAGIC_VALUE 0xDEADBEEF

namespace network {

#pragma pack(push, 1)

template <typename T>
struct message_header {
    uint32_t magic_value = MAGIC_VALUE;
    uint32_t user_id = 0;
    uint32_t tick = 0;
    T id{};
    uint32_t size = 0;
    message_header() : magic_value(MAGIC_VALUE) {}
};

#pragma pack(pop)

template <typename T>
struct message {
    message_header<T> header{};

    std::vector<uint8_t> body;

    size_t size() const { return body.size(); }

    friend std::ostream& operator<<(std::ostream& os, const message<T>& msg) {
        os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
        return os;
    }

    template <typename DataType>
    friend message<T>& operator<<(message<T>& msg, const DataType& data) {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

        size_t i = msg.body.size();

        msg.body.resize(msg.body.size() + sizeof(DataType));
        std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
        msg.header.size = (uint32_t)msg.size();

        return msg;
    }

    friend message<T>& operator<<(message<T>& msg, const std::vector<uint8_t>& data) {
        size_t i = msg.body.size();
        msg.body.resize(msg.body.size() + data.size());
        std::memcpy(msg.body.data() + i, data.data(), data.size());
        msg.header.size = (uint32_t)msg.size();
        return msg;
    }

    template <typename DataType>
    friend message<T>& operator>>(message<T>& msg, DataType& data) {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

        if (msg.body.size() < sizeof(DataType)) {
            throw std::runtime_error("Message body too small for pop: " + std::to_string(msg.body.size()) + " < " +
                                     std::to_string(sizeof(DataType)));
        }

        size_t i = msg.body.size() - sizeof(DataType);

        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
        msg.body.resize(i);
        msg.header.size = (uint32_t)msg.size();

        return msg;
    }

    void to_little_endian() {
        auto swap_uint32 = [](uint32_t val) -> uint32_t {
            return ((val >> 24) & 0x000000FF) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) |
                   ((val << 24) & 0xFF000000);
        };

        auto swap_t = [](T val) -> T {
            if constexpr (sizeof(T) == 1)
                return val;
            if constexpr (sizeof(T) == 2) {
                uint16_t x = static_cast<uint16_t>(val);
                x = (x >> 8) | (x << 8);
                return static_cast<T>(x);
            }
            if constexpr (sizeof(T) == 4) {
                uint32_t x = static_cast<uint32_t>(val);
                x = ((x >> 24) & 0x000000FF) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) |
                    ((x << 24) & 0xFF000000);
                return static_cast<T>(x);
            }
            // Fallback for other sizes if necessary, or just return val for now to avoid errors on complex types
            return val;
        };

        header.magic_value = swap_uint32(header.magic_value);
        header.user_id = swap_uint32(header.user_id);
        header.tick = swap_uint32(header.tick);
        header.id = swap_t(header.id);
        header.size = swap_uint32(header.size);
    }
};

template <typename T>
class Connection;

template <typename T>
struct owned_message {
    std::shared_ptr<Connection<T>> remote = nullptr;
    message<T> msg;

    friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg) {
        os << msg.msg;
        return os;
    }
};
}  // namespace network