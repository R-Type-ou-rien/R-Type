#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "PlayerBoundsSystem.hpp"

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
    
    enum BossState {
        SPAWN,
        PHASE_1,
        PHASE_2,
        PHASE_3,
        ENRAGED,
        DYING,
        DEAD
    };
    
    bool has_arrived = false;
    float target_x = 0.0f;
    
    // Machine à états
    BossState current_state = SPAWN;
    float state_timer = 0.0f;
    
    // Système de phases
    int current_phase = 1;
    int max_phases = 3;
    float phase_transition_timer = 0.0f;
    
    // Patterns d'attaque
    float attack_pattern_timer = 0.0f;
    float attack_pattern_interval = 3.0f;
    int current_attack_pattern = 0;
    
    // Mouvement
    float oscillation_timer = 0.0f;
    float oscillation_amplitude = 100.0f;
    float oscillation_frequency = 1.0f;
    float base_y = 0.0f;
    
    // Points faibles
    bool has_weak_points = true;
    int weak_points_destroyed = 0;
    int total_weak_points = 2;
    bool core_vulnerable = true;
    
    // Phase ENRAGED
    bool is_enraged = false;
    
    // Dying sequence
    float death_timer = 0.0f;
    float death_duration = 3.0f;
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
