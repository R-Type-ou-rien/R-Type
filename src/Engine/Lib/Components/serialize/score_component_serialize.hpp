#pragma once

#include "../../../../RType/Common/Systems/score.hpp"
#include "serialize.hpp"
#include <vector>

namespace serialize {

/** ScoreComponent */
inline void serialize(std::vector<uint8_t>& buffer, const ScoreComponent& component) {
    serialize(buffer, component.current_score);
    serialize(buffer, component.high_score);
}

inline ScoreComponent deserialize_score_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    ScoreComponent component;
    component.current_score = deserialize<int>(buffer, offset);
    component.high_score = deserialize<int>(buffer, offset);
    return component;
}

}  // namespace serialize
