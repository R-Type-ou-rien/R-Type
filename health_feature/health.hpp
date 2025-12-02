#pragma once
#include <iostream>

class Registry;

namespace ECS {
struct HealthComponent {
    int max_hp;
    int current_hp;
};

class HealthSystem {
   public:
    HealthSystem() = default;
    ~HealthSystem() = default;
    void update(Registry& registry);

   private:
    void check_death(Registry& registry); //, Entity e);
    void handle_enemy_death(Registry& registry); //, Entity e);
    void handle_player_death(Registry& registry); //, Entity e);
};
}  // namespace ECS
