#pragma once

#include "ECS/Registry/registry.hpp"
#include "../Components/LobbyIdComponent.hpp"

namespace engine {
namespace utils {

inline uint32_t getLobbyId(Registry& registry, Entity entity) {
    if (registry.hasComponent<LobbyIdComponent>(entity)) {
        return registry.getConstComponent<LobbyIdComponent>(entity).lobby_id;
    }
    return 0;
}

}  // namespace utils
}  // namespace engine
