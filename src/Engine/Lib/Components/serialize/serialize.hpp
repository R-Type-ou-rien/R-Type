#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <tuple>
#include <algorithm>

namespace serialize {

/** Basic serializer */
template <typename T>
void serialize(std::vector<uint8_t>& buffer, const T& value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
}

template <>
inline void serialize<std::string>(std::vector<uint8_t>& buffer, const std::string& str) {
    serialize(buffer, static_cast<uint32_t>(str.size()));
    buffer.insert(buffer.end(), str.begin(), str.end());
}

template <typename T>
void serialize(std::vector<uint8_t>& buffer, const std::vector<T>& vec) {
    serialize(buffer, static_cast<uint32_t>(vec.size()));
    for (const auto& element : vec) {
        serialize(buffer, element);
    }
}

template <typename K, typename V>
void serialize(std::vector<uint8_t>& buffer, const std::map<K, V>& map) {
    serialize(buffer, static_cast<uint32_t>(map.size()));
    for (const auto& pair : map) {
        serialize(buffer, pair.first);
        serialize(buffer, pair.second);
    }
}

template <typename... Args>
void serialize(std::vector<uint8_t>& buffer, const std::tuple<Args...>& t) {
    std::apply([&](const Args&... args) { (serialize(buffer, args), ...); }, t);
}

/** Basic Deserializer */
template <typename T>
T deserialize(const std::vector<uint8_t>& buffer, size_t& offset) {
    T value;
    std::copy(buffer.begin() + offset, buffer.begin() + offset + sizeof(T), reinterpret_cast<uint8_t*>(&value));
    offset += sizeof(T);
    return value;
}

template <>
inline std::string deserialize<std::string>(const std::vector<uint8_t>& buffer, size_t& offset) {
    uint32_t size = deserialize<uint32_t>(buffer, offset);
    std::string str(buffer.begin() + offset, buffer.begin() + offset + size);
    offset += size;
    return str;
}

template <typename T>
std::vector<T> deserialize_vector(const std::vector<uint8_t>& buffer, size_t& offset) {
    uint32_t size = deserialize<uint32_t>(buffer, offset);
    std::vector<T> vec;
    vec.reserve(size);
    for (uint32_t i = 0; i < size; ++i) {
        vec.push_back(deserialize<T>(buffer, offset));
    }
    return vec;
}

template <typename K, typename V>
std::map<K, V> deserialize_map(const std::vector<uint8_t>& buffer, size_t& offset) {
    uint32_t size = deserialize<uint32_t>(buffer, offset);
    std::map<K, V> map;
    for (uint32_t i = 0; i < size; ++i) {
        K key = deserialize<K>(buffer, offset);
        V value = deserialize<V>(buffer, offset);
        map[key] = value;
    }
    return map;
}

template <typename... Args>
std::tuple<Args...> deserialize_tuple(const std::vector<uint8_t>& buffer, size_t& offset) {
    return std::make_tuple(deserialize<Args>(buffer, offset)...);
}

}  // namespace serialize
