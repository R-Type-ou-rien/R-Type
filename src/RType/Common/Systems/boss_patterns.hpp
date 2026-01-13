#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "ai_behavior.hpp"
#include "../Components/boss_component.hpp"

class BossPatternSystem : public ISystem {
   public:
    BossPatternSystem() = default;
    ~BossPatternSystem() = default;
    void update(Registry& registry, system_context context) override;
    
   private:
    void updateBossState(Registry& registry, system_context context, Entity boss_entity);
    void handlePhaseTransition(Registry& registry, Entity boss_entity, BossComponent& boss);
    
    // Patterns par phase
    void executePhase1Patterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss);
    void executePhase2Patterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss);
    void executePhase3Patterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss);
    void executeEnragedPatterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss);
    
    // Patterns spécifiques
    void patternLinearAlternate(Registry& registry, system_context context, Entity boss_entity);
    void patternSlowMissiles(Registry& registry, system_context context, Entity boss_entity);
    void patternWallOfProjectiles(Registry& registry, system_context context, Entity boss_entity);
    void patternBouncingShots(Registry& registry, system_context context, Entity boss_entity);
    void patternSpiral(Registry& registry, system_context context, Entity boss_entity);
    void patternDelayedShots(Registry& registry, system_context context, Entity boss_entity);
    
    // Gestion des sous-entités
    void updateSubEntities(Registry& registry, system_context context);
    void spawnTentacle(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y);
    void spawnCannon(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y);
    
    // Création de projectiles boss
    void createBossProjectile(Registry& registry, system_context context, 
                              const transform_component_s& pos, float vx, float vy, int damage);
};
