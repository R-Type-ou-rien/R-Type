#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct HealthComponent {
    int max_hp;
    int current_hp;
};

class HealthSystem : public ISystem {
   public:
    HealthSystem() = default;
    ~HealthSystem() = default;
    void update(Registry& registry, system_context context) override;
};
