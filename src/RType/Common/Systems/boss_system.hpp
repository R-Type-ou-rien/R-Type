#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "behavior.hpp"
#include <functional>
#include <map>
#include <vector>
#include "Components/StandardComponents.hpp"
#include <string>
#include "../Components/boss_component.hpp"

class BossSystem : public ISystem {
   public:
    BossSystem();
    ~BossSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    using StateHandler = std::function<void(Registry&, system_context, Entity, BossComponent&)>;
    using TransitionHandler = std::function<void(Registry&, Entity, BossComponent&)>;
    using PatternHandler = std::function<void(Registry&, system_context, Entity)>;

    std::map<BossComponent::BossState, StateHandler> _state_handlers;
    std::map<BossComponent::BossState, TransitionHandler> _transition_handlers;
    std::map<std::string, PatternHandler> _available_patterns;

    void initializeHandlers();
    void registerPatternHandler(const std::string& name, PatternHandler handler);
    void executePatterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss,
                         const std::vector<std::string>& pattern_names);
    void updateBossState(Registry& registry, system_context context, Entity boss_entity);
    void checkArrival(Registry& registry, Entity boss_entity, BossComponent& boss);
    BossComponent::BossState getNextState(const BossComponent& boss, float health_percent);
    void executePatternLogic(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss,
                             const std::vector<PatternHandler>& patterns);
    void spawnSubEntitiesRange(Registry& registry, Entity boss_entity, const BossComponent& boss, size_t start_idx,
                               size_t count);
    void patternLinearAlternate(Registry& registry, system_context context, Entity boss_entity);
    void patternSlowMissiles(Registry& registry, system_context context, Entity boss_entity);
    void patternWallOfProjectiles(Registry& registry, system_context context, Entity boss_entity);
    void patternBouncingShots(Registry& registry, system_context context, Entity boss_entity);
    void patternSpiral(Registry& registry, system_context context, Entity boss_entity);
    void patternDelayedShots(Registry& registry, system_context context, Entity boss_entity);
    void patternOrbitalAim(Registry& registry, system_context context, Entity boss_entity);
    void updateSubEntities(Registry& registry, system_context context);
    void spawnTentacle(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y,
                       float fire_rate);
    void spawnCannon(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y,
                     float fire_rate);
    void createBossProjectile(Registry& registry, system_context context, const transform_component_s& pos, float vx,
                              float vy, int damage, uint32_t lobbyId = 0);
};
