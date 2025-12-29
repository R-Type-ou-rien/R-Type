#pragma once

#include "registry.hpp"

struct CollidedEntity {
    static constexpr std::string_view name = "CollidedEntity";
    std::vector<Entity> tags;
};
