#pragma once

#include <vector>

#include "../../Engine/Core/ECS/Registry/registry.hpp"

struct TagComponent {
    std::vector<Entity> tags;
};
