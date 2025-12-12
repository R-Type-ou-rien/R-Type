#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct DamageOnCollision {
    int damage_value;
};

class Damage : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
