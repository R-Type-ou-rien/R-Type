#pragma once

#include "ecs/Registry/registry.hpp"

struct TagComponent {
    std::vector<Entity> tags;
};
