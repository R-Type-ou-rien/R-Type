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
    T id{};
    uint32_t size = 0;
    message_header<T>() : magic_value(MAGIC_VALUE) {}
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

    template <typename DataType>
    friend message<T>& operator>>(message<T>& msg, DataType& data) {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

        size_t i = msg.body.size() - sizeof(DataType);

        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
        msg.body.resize(i);
        msg.header.size = (uint32_t)msg.size();

        return msg;
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