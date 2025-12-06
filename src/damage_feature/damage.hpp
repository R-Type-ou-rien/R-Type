#pragma once

#include "ecs/Registry/registry.hpp"
#include "shoot_feature/shooter.hpp"
#include "../health_feature/health.hpp"

struct DamageOnColision {
    int damage_value;
};

class Damage {
    public:
        void update(Registry& registry, double time_now); 
};
