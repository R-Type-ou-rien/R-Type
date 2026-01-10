#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct DamageOnCollision {
    static constexpr auto name = "DamageOnCollision";
    int damage_value;
};

class Damage : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};

class HealthSystem : public ISystem {
   public:
    HealthSystem() = default;
    ~HealthSystem() = default;
    void update(Registry& registry, system_context context) override;
};
