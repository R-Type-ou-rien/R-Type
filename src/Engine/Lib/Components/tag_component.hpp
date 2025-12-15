#pragma once

#include "registry.hpp"

struct CollidedEntity {
    static constexpr auto name = "CollidedEntity";
    std::vector<Entity> tags;
};
