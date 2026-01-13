#pragma once

#include <vector>
#include "Components/tag_component.hpp"
#include "serialize.hpp"

namespace serialize {

/** CollidedEntity Component */
inline void serialize(std::vector<uint8_t>& buffer, const CollidedEntity& component) {
    serialize(buffer, static_cast<uint32_t>(component.tags.size()));
    for (const auto& entity : component.tags) {
        serialize(buffer, entity);
    }
}

inline CollidedEntity deserialize_collided_entity(const std::vector<uint8_t>& buffer, size_t& offset) {
    CollidedEntity component;
    component.tags = deserialize_vector<Entity>(buffer, offset);
    return component;
}

}  // namespace serialize
