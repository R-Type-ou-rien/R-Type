#pragma once

#include "ecs/Registry/registry.hpp"
#include "ecs/System/ISystem.hpp"

struct DamageOnCollision {
    int damage_value;
};

class Damage : public ISystem {
    public:
        void update(Registry& registry, system_context context) override;
        void init(Registry& registry) override {};
};
