#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct HealthComponent {
    static constexpr auto name = "Health";
    int max_hp = 100;
    int current_hp = 100;
    float last_damage_time = 0.0f;
    float invincibility_duration = 1.0f;
};

class HealthSystem : public ISystem {
   public:
    HealthSystem() = default;
    ~HealthSystem() = default;
    void update(Registry& registry, system_context context) override;
};
