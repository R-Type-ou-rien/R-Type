#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct DamageOnCollision {
    static constexpr auto name = "Damage";
    int damage_value = 10;
};

class Damage : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
