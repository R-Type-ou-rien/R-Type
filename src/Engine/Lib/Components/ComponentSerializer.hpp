#pragma once

#include <vector>
#include <cstring>
#include <map>
#include <string>
#include <iostream>
#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"

// Helper to write POD data to vector
template <typename T>
void WritePOD(std::vector<uint8_t>& buffer, const T& data) {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&data);
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

// Helper to read POD data from vector
template <typename T>
void ReadPOD(const std::vector<uint8_t>& buffer, size_t& offset, T& data) {
    if (offset + sizeof(T) > buffer.size())
        return;
    std::memcpy(&data, buffer.data() + offset, sizeof(T));
    offset += sizeof(T);
}

template <typename T>
struct Serializer {
    static std::vector<uint8_t> serialize(const T& component) {
        std::vector<uint8_t> data;
        WritePOD(data, component);  // Default fallback: POD copy
        return data;
    }

    static void deserialize(T& component, const std::vector<uint8_t>& data) {
        if (data.size() != sizeof(T))
            return;
        std::memcpy(&component, data.data(), sizeof(T));
    }
};

// PatternComponent Serialization (contains vector)
template <>
struct Serializer<PatternComponent> {
    static std::vector<uint8_t> serialize(const PatternComponent& component) {
        std::vector<uint8_t> data;
        // Waypoints (vector size + elements)
        uint32_t vecSize = component.waypoints.size();
        WritePOD(data, vecSize);
        for (const auto& wp : component.waypoints) {
            WritePOD(data, wp.first);
            WritePOD(data, wp.second);
        }
        WritePOD(data, component.current_index);
        WritePOD(data, component.speed);
        WritePOD(data, component.loop);
        WritePOD(data, component.is_active);
        return data;
    }

    static void deserialize(PatternComponent& component, const std::vector<uint8_t>& data) {
        size_t offset = 0;
        uint32_t vecSize = 0;
        ReadPOD(data, offset, vecSize);
        component.waypoints.resize(vecSize);
        for (uint32_t i = 0; i < vecSize; ++i) {
            float x, y;
            ReadPOD(data, offset, x);
            ReadPOD(data, offset, y);
            component.waypoints[i] = {x, y};
        }
        ReadPOD(data, offset, component.current_index);
        ReadPOD(data, offset, component.speed);
        ReadPOD(data, offset, component.loop);
        ReadPOD(data, offset, component.is_active);
    }
};

// TagComponent Serialization (contains vector of strings)
template <>
struct Serializer<TagComponent> {
    static std::vector<uint8_t> serialize(const TagComponent& component) {
        std::vector<uint8_t> data;
        uint32_t vecSize = component.tags.size();
        WritePOD(data, vecSize);
        for (const auto& tag : component.tags) {
            uint32_t strSize = tag.size();
            WritePOD(data, strSize);
            data.insert(data.end(), tag.begin(), tag.end());
        }
        return data;
    }

    static void deserialize(TagComponent& component, const std::vector<uint8_t>& data) {
        size_t offset = 0;
        uint32_t vecSize = 0;
        ReadPOD(data, offset, vecSize);
        component.tags.resize(vecSize);
        for (uint32_t i = 0; i < vecSize; ++i) {
            uint32_t strSize = 0;
            ReadPOD(data, offset, strSize);
            if (offset + strSize > data.size())
                return;
            component.tags[i].assign(reinterpret_cast<const char*>(data.data() + offset), strSize);
            offset += strSize;
        }
    }
};

// ActionScript: SKIP (contains functions and maps)
template <>
struct Serializer<ActionScript> {
    static std::vector<uint8_t> serialize(const ActionScript& component) {
        // Cannot serialize functions
        return {};
    }

    static void deserialize(ActionScript& component, const std::vector<uint8_t>& data) {
        // Do nothing
    }
};

// BoxCollisionComponent: SKIP (contains function)
template <>
struct Serializer<BoxCollisionComponent> {
    static std::vector<uint8_t> serialize(const BoxCollisionComponent& component) { return {}; }

    static void deserialize(BoxCollisionComponent& component, const std::vector<uint8_t>& data) {}
};

// ResourceComponent (contains map)
template <>
struct Serializer<ResourceComponent> {
    static std::vector<uint8_t> serialize(const ResourceComponent& component) {
        std::vector<uint8_t> data;
        uint32_t mapSize = component.resources.size();
        WritePOD(data, mapSize);
        for (const auto& kv : component.resources) {
            uint32_t keySize = kv.first.size();
            WritePOD(data, keySize);
            data.insert(data.end(), kv.first.begin(), kv.first.end());
            WritePOD(data, kv.second);  // ResourceStat is POD
        }
        return data;
    }

    static void deserialize(ResourceComponent& component, const std::vector<uint8_t>& data) {
        size_t offset = 0;
        uint32_t mapSize = 0;
        ReadPOD(data, offset, mapSize);
        for (uint32_t i = 0; i < mapSize; ++i) {
            uint32_t keySize = 0;
            ReadPOD(data, offset, keySize);
            if (offset + keySize > data.size())
                return;
            std::string key(reinterpret_cast<const char*>(data.data() + offset), keySize);
            offset += keySize;
            ResourceStat stat;
            ReadPOD(data, offset, stat);
            component.resources[key] = stat;
        }
    }
};

// NetworkIdentity Serialization (POD)
template <>
struct Serializer<NetworkIdentity> {
    static std::vector<uint8_t> serialize(const NetworkIdentity& component) {
        std::vector<uint8_t> data;
        WritePOD(data, component);
        return data;
    }

    static void deserialize(NetworkIdentity& component, const std::vector<uint8_t>& data) {
        if (data.size() != sizeof(NetworkIdentity))
            return;
        std::memcpy(&component, data.data(), sizeof(NetworkIdentity));
    }
};

// ComponentPacket Serialization (Should rely on its own operator<<)
template <>
struct Serializer<ComponentPacket> {
    static std::vector<uint8_t> serialize(const ComponentPacket& component) {
        // Component packet shouldn't be inside another component packet usually
        return {};
    }
    static void deserialize(ComponentPacket& component, const std::vector<uint8_t>& data) {}
};
