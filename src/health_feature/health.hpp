#pragma once

#include "ecs/Registry/registry.hpp"
#include "ecs/System/ISystem.hpp"

struct HealthComponent {
    int max_hp;
    int current_hp;
};

struct LifeComponent {
    int lives_remaining;
};

class HealthSystem : public ISystem {
   public:
    HealthSystem() = default;
    ~HealthSystem() = default;
    void update(Registry& registry, system_context context) override;
    void init(Registry& registry) override {};
};
