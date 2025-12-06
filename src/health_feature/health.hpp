#pragma once
#include <iostream>

#include "ecs/Registry/registry.hpp"

struct HealthComponent {
    int max_hp;
    int current_hp;
};

struct LifeComponent {
    int lives_remaining;
};

class HealthSystem {
   public:
    HealthSystem() = default;
    ~HealthSystem() = default;
    void update(Registry& registry, double time_now);
};
