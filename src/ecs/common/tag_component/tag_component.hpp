#pragma once

#include "ecs/common/Registry/registry.hpp"

struct TagComponent {
    std::vector<Entity> tags;
};
