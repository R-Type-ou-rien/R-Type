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
        void update();

    private:
        void check_death(Entity e);
        void handle_enemy_death(Entity e);
        void handle_player_death(Entity e);
    }
}
