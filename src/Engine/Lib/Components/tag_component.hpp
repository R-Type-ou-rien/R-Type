#pragma once

#include <vector>
#include "EcsType.hpp"

struct CollidedEntity {
    static constexpr auto name = "CollidedEntity";
    std::vector<Entity> tags;
};
