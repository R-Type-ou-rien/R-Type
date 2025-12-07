#pragma once

#include <vector>

#include "ecs/Registry/registry.hpp"

struct TagComponent {
    std::vector<Entity> tags;
};
