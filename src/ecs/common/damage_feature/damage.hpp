#pragma once

#include "ecs/common/ISystem.hpp"
#include "ecs/common/Registry/registry.hpp"

struct DamageOnCollision {
    int damage_value;
};

class Damage : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
