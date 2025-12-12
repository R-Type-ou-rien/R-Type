#pragma once

#include <vector>

#include "ecs/common/Registry/registry.hpp"

struct TagComponent {
    std::vector<Entity> tags;
};
