#pragma once

#include <SFML/System/Vector2.hpp>

#include "ecs/Registry/registry.hpp"

struct TransformComponent {
    float x;
    float y;
    float rotation = 0;
    sf::Vector2f scale = {1.0, 1.0};
};
