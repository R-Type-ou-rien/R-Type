#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "damage_component.hpp"

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
