#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

// Composant pour les ennemis qui visent le joueur
struct AIBehaviorComponent {
    static constexpr auto name = "AIBehaviorComponent";
    bool shoot_at_player = false;
    bool follow_player = false;
    float follow_speed = 100.0f;
};

// Composant pour marquer le boss (utile pour arrêter le background)
struct BossComponent {
    static constexpr auto name = "BossComponent";
    bool has_arrived = false;
    float target_x = 0.0f;
};

// Composant pour les limites du monde
struct WorldBoundsComponent {
    static constexpr auto name = "WorldBoundsComponent";
    float min_x = 0.0f;
    float max_x = 1920.0f;
    float min_y = 0.0f;
    float max_y = 1080.0f;
};

class AIBehaviorSystem : public ISystem {
public:
    AIBehaviorSystem() = default;
    ~AIBehaviorSystem() = default;
    void update(Registry& registry, system_context context) override;
    
private:
    void updateFollowPlayer(Registry& registry, system_context context, Entity enemy, Entity player);
    void updateShootAtPlayer(Registry& registry, Entity enemy, Entity player);
};

class BoundsSystem : public ISystem {
public:
    BoundsSystem() = default;
    ~BoundsSystem() = default;
    void update(Registry& registry, system_context context) override;
};
