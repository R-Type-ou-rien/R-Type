#include "boss_system.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/team_component.hpp"
#include "../Components/behavior_component.hpp"
#include "shooter.hpp"
#include "health.hpp"
#include "damage.hpp"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include "../../../../Engine/Lib/Components/LobbyIdComponent.hpp"

BossSystem::BossSystem() {
    initializeHandlers();
}

void BossSystem::initializeHandlers() {
    _state_handlers[BossComponent::SPAWN] = [](Registry&, system_context, Entity, BossComponent&) {};

    _state_handlers[BossComponent::PHASE_1] = [this](Registry& registry, system_context context, Entity entity,
                                                     BossComponent& boss) {
        executePatternLogic(registry, context, entity, boss, _phase1_patterns);
    };

    _state_handlers[BossComponent::PHASE_2] = [this](Registry& registry, system_context context, Entity entity,
                                                     BossComponent& boss) {
        executePatternLogic(registry, context, entity, boss, _phase2_patterns);
    };

    _state_handlers[BossComponent::PHASE_3] = [this](Registry& registry, system_context context, Entity entity,
                                                     BossComponent& boss) {
        executePatternLogic(registry, context, entity, boss, _phase3_patterns);
    };

    _state_handlers[BossComponent::ENRAGED] = [this](Registry& registry, system_context context, Entity entity,
                                                     BossComponent& boss) {
        executePatternLogic(registry, context, entity, boss, _enraged_patterns);
    };

    _state_handlers[BossComponent::DYING] = [](Registry& registry, system_context context, Entity entity,
                                               BossComponent& boss) {
        boss.death_timer += context.dt;
        if (boss.death_timer > 3.0f) {
            boss.current_state = BossComponent::DEAD;
        }
    };

    _state_handlers[BossComponent::DEAD] = [](Registry& registry, system_context, Entity entity, BossComponent&) {
        if (!registry.hasComponent<PendingDestruction>(entity)) {
            registry.addComponent<PendingDestruction>(entity, {});
        }
    };

    // Transition Handlers
    _transition_handlers[BossComponent::PHASE_1] = [](Registry&, Entity, BossComponent& boss) {
        boss.attack_pattern_interval = 2.0f;
    };

    _transition_handlers[BossComponent::PHASE_2] = [this](Registry& registry, Entity entity, BossComponent& boss) {
        boss.attack_pattern_interval = 1.5f;
        spawnSubEntitiesRange(registry, entity, boss, 0, 2);
    };

    _transition_handlers[BossComponent::PHASE_3] = [this](Registry& registry, Entity entity, BossComponent& boss) {
        boss.attack_pattern_interval = 1.0f;
        spawnSubEntitiesRange(registry, entity, boss, 2, 2);
    };

    _transition_handlers[BossComponent::ENRAGED] = [](Registry&, Entity, BossComponent& boss) {
        boss.attack_pattern_interval = 0.5f;
    };

    _transition_handlers[BossComponent::DYING] = [](Registry&, Entity, BossComponent&) {};

    // Patterns
    _phase1_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternLinearAlternate(r, c, e); });
    _phase1_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternSlowMissiles(r, c, e); });

    _phase2_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternWallOfProjectiles(r, c, e); });
    _phase2_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternBouncingShots(r, c, e); });

    _phase3_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternSpiral(r, c, e); });
    _phase3_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternDelayedShots(r, c, e); });

    _enraged_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternSpiral(r, c, e); });
    _enraged_patterns.push_back([this](Registry& r, system_context c, Entity e) { patternWallOfProjectiles(r, c, e); });
}

void BossSystem::update(Registry& registry, system_context context) {
    auto& bosses = registry.getEntities<BossComponent>();

    for (auto boss_entity : bosses) {
        updateBossState(registry, context, boss_entity);
    }

    updateSubEntities(registry, context);
}

void BossSystem::updateBossState(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<BossComponent>(boss_entity))
        return;

    auto& boss = registry.getComponent<BossComponent>(boss_entity);

    checkArrival(registry, boss_entity, boss);

    if (boss.damage_flash_timer > 0.0f) {
        boss.damage_flash_timer -= context.dt;
        if (boss.damage_flash_timer < 0.0f) {
            boss.damage_flash_timer = 0.0f;
        }
    }

    if (!registry.hasComponent<HealthComponent>(boss_entity)) {
        return;
    }

    auto& health = registry.getComponent<HealthComponent>(boss_entity);
    float health_percent = static_cast<float>(health.current_hp) / static_cast<float>(health.max_hp);

    // Gestion prioritaire de la mort
    if (health.current_hp <= 0 && boss.current_state != BossComponent::DYING &&
        boss.current_state != BossComponent::DEAD) {
        if (_transition_handlers.count(BossComponent::DYING)) {
            _transition_handlers[BossComponent::DYING](registry, boss_entity, boss);
        }
        boss.current_state = BossComponent::DYING;
        boss.death_timer = 0.0f;
        boss.state_timer = 0.0f;
        return;
    }

    BossComponent::BossState target_state = getNextState(boss, health_percent);

    if (target_state != boss.current_state) {
        // Exécuter la logique d'entrée du NOUVEL état
        if (_transition_handlers.count(target_state)) {
            _transition_handlers[target_state](registry, boss_entity, boss);
        }
        boss.current_state = target_state;
        boss.state_timer = 0.0f;
    }

    boss.state_timer += context.dt;

    // Exécuter la logique de l'état courant
    if (_state_handlers.count(boss.current_state)) {
        _state_handlers[boss.current_state](registry, context, boss_entity, boss);
    }
}

void BossSystem::checkArrival(Registry& registry, Entity boss_entity, BossComponent& boss) {
    if (boss.has_arrived || !registry.hasComponent<transform_component_s>(boss_entity))
        return;

    auto& transform = registry.getComponent<transform_component_s>(boss_entity);
    if (transform.x <= boss.target_x) {
        transform.x = boss.target_x;
        boss.has_arrived = true;
        if (registry.hasComponent<Velocity2D>(boss_entity)) {
            auto& vel = registry.getComponent<Velocity2D>(boss_entity);
            vel.vx = 0.0f;
            vel.vy = 0.0f;
        }
    }
}

BossComponent::BossState BossSystem::getNextState(const BossComponent& boss, float health_percent) {
    if (health_percent <= 0.0f)
        return BossComponent::DEAD;

    // Logique de transition séquentielle
    if (health_percent <= 0.10f && boss.current_state != BossComponent::ENRAGED)
        return BossComponent::ENRAGED;
    if (health_percent <= 0.40f && boss.current_state == BossComponent::PHASE_2)
        return BossComponent::PHASE_3;
    if (health_percent <= 0.75f && boss.current_state == BossComponent::PHASE_1)
        return BossComponent::PHASE_2;
    if (boss.has_arrived && boss.current_state == BossComponent::SPAWN)
        return BossComponent::PHASE_1;

    return boss.current_state;
}

void BossSystem::spawnSubEntitiesRange(Registry& registry, Entity boss_entity, const BossComponent& boss,
                                       size_t start_idx, size_t count) {
    size_t end_idx = std::min(start_idx + count, boss.sub_entities_config.size());

    for (size_t i = start_idx; i < end_idx; ++i) {
        const auto& cfg = boss.sub_entities_config[i];
        if (cfg.type == "TENTACLE") {
            spawnTentacle(registry, boss_entity, static_cast<int>(i), cfg.offset_x, cfg.offset_y, cfg.fire_rate);
        } else if (cfg.type == "CANNON") {
            spawnCannon(registry, boss_entity, static_cast<int>(i), cfg.offset_x, cfg.offset_y, cfg.fire_rate);
        }
    }
}

void BossSystem::executePatternLogic(Registry& registry, system_context context, Entity boss_entity,
                                     BossComponent& boss, const std::vector<PatternHandler>& patterns) {
    if (patterns.empty())
        return;

    boss.attack_pattern_timer += context.dt;

    if (boss.attack_pattern_timer >= boss.attack_pattern_interval) {
        boss.attack_pattern_timer = 0.0f;

        // Sélection cyclique du pattern
        size_t pattern_index = boss.current_attack_pattern % patterns.size();
        patterns[pattern_index](registry, context, boss_entity);

        boss.current_attack_pattern++;
    }
}

// Patterns spécifiques
void BossSystem::patternLinearAlternate(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    // Trouver les tentacules actives
    auto& sub_entities = registry.getEntities<BossSubEntityComponent>();
    for (auto sub_entity : sub_entities) {
        auto& sub = registry.getComponent<BossSubEntityComponent>(sub_entity);

        if (!sub.is_active || sub.is_destroyed)
            continue;
        if (sub.type != BossSubEntityComponent::TENTACLE)
            continue;
        if (sub.boss_entity_id != boss_entity)
            continue;

        auto& boss = registry.getConstComponent<BossComponent>(boss_entity);
        uint32_t lobbyId = 0;
        if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
            lobbyId = registry.getComponent<LobbyIdComponent>(boss_entity).lobby_id;
        }
        // Tir linéaire droit vers la gauche
        if (registry.hasComponent<transform_component_s>(sub_entity)) {
            auto& sub_transform = registry.getConstComponent<transform_component_s>(sub_entity);
            createBossProjectile(registry, context, sub_transform, boss.patterns.linear_speed, 0.0f,
                                 boss.patterns.linear_damage, lobbyId);
        }
    }
}

void BossSystem::patternSlowMissiles(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;

    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    auto& boss = registry.getConstComponent<BossComponent>(boss_entity);

    // 2 missiles lents
    uint32_t lobbyId = 0;
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        lobbyId = registry.getComponent<LobbyIdComponent>(boss_entity).lobby_id;
    }
    createBossProjectile(registry, context, boss_transform, boss.patterns.missile_speed,
                         -boss.patterns.missile_offset_y, boss.patterns.missile_damage, lobbyId);
    createBossProjectile(registry, context, boss_transform, boss.patterns.missile_speed, boss.patterns.missile_offset_y,
                         boss.patterns.missile_damage, lobbyId);
}

void BossSystem::patternWallOfProjectiles(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;

    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    auto& boss = registry.getConstComponent<BossComponent>(boss_entity);

    // Éventail de 5 projectiles
    uint32_t lobbyId = 0;
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        lobbyId = registry.getComponent<LobbyIdComponent>(boss_entity).lobby_id;
    }
    for (int i = -boss.patterns.wall_count_side; i <= boss.patterns.wall_count_side; i++) {
        float angle = i * boss.patterns.wall_angle_step;
        float vx = boss.patterns.wall_speed * std::cos(angle * 3.14159f / 180.0f);
        float vy = boss.patterns.wall_speed * std::sin(angle * 3.14159f / 180.0f);
        createBossProjectile(registry, context, boss_transform, vx, vy, boss.patterns.wall_damage, lobbyId);
    }
}

void BossSystem::patternBouncingShots(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;

    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    auto& boss = registry.getConstComponent<BossComponent>(boss_entity);

    uint32_t lobbyId = 0;
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        lobbyId = registry.getComponent<LobbyIdComponent>(boss_entity).lobby_id;
    }
    createBossProjectile(registry, context, boss_transform, boss.patterns.bounce_speed_x,
                         -boss.patterns.bounce_offset_y, boss.patterns.bounce_damage, lobbyId);
    createBossProjectile(registry, context, boss_transform, boss.patterns.bounce_speed_x, boss.patterns.bounce_offset_y,
                         boss.patterns.bounce_damage, lobbyId);
}

void BossSystem::patternSpiral(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    if (!registry.hasComponent<BossComponent>(boss_entity))
        return;

    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    auto& boss = registry.getConstComponent<BossComponent>(boss_entity);

    // Spirale basée sur le timer
    float angle = boss.state_timer * boss.patterns.spiral_rotation_speed;
    int projectile_count = boss.patterns.spiral_count;

    uint32_t lobbyId = 0;
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        lobbyId = registry.getComponent<LobbyIdComponent>(boss_entity).lobby_id;
    }

    for (int i = 0; i < projectile_count; i++) {
        float current_angle = (angle + (i * 360.0f / projectile_count)) * 3.14159f / 180.0f;
        float vx = boss.patterns.spiral_speed * std::cos(current_angle);
        float vy = boss.patterns.spiral_speed * std::sin(current_angle);
        createBossProjectile(registry, context, boss_transform, vx, vy, boss.patterns.spiral_damage, lobbyId);
    }
}

void BossSystem::patternDelayedShots(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;

    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    auto& boss = registry.getConstComponent<BossComponent>(boss_entity);
    uint32_t lobbyId = 0;
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        lobbyId = registry.getComponent<LobbyIdComponent>(boss_entity).lobby_id;
    }
    createBossProjectile(registry, context, boss_transform, boss.patterns.delayed_speed, 0.0f,
                         boss.patterns.delayed_damage, lobbyId);
}

// Création de projectiles du boss
// Création de projectiles du boss
void BossSystem::createBossProjectile(Registry& registry, system_context context, const transform_component_s& pos,
                                      float vx, float vy, int damage, uint32_t lobbyId) {
    Entity projectile = registry.createEntity();

    // Team ennemi
    registry.addComponent<TeamComponent>(projectile, {TeamComponent::ENEMY});

    // Tag pour collision avec le joueur
    TagComponent tags;
    tags.tags.push_back("ENEMY_PROJECTILE");
    registry.addComponent<TagComponent>(projectile, tags);

    // Marquer comme projectile
    registry.addComponent<ProjectileComponent>(projectile, {});

    // Position et vitesse
    registry.addComponent<transform_component_s>(projectile, {pos.x + BossDefaults::Projectile::OFFSET_X, pos.y});
    registry.addComponent<Velocity2D>(projectile, {vx, vy});

    // Dégâts
    registry.addComponent<DamageOnCollision>(projectile, {damage});

    // Sprite visible (projectile ennemi rouge)
    handle_t<TextureAsset> handle = context.texture_manager.load(BossDefaults::Projectile::SPRITE_PATH,
                                                                 TextureAsset(BossDefaults::Projectile::SPRITE_PATH));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0.0f;
    sprite_info.current_animation_frame = 0;
    // Projectile ennemi (boule d'énergie rouge)
    sprite_info.dimension = {BossDefaults::Projectile::SPRITE_X, BossDefaults::Projectile::SPRITE_Y,
                             BossDefaults::Projectile::SPRITE_W, BossDefaults::Projectile::SPRITE_H};
    sprite_info.z_index = BossDefaults::Projectile::Z_INDEX;
    registry.addComponent<sprite2D_component_s>(projectile, sprite_info);

    // Échelle pour rendre visible
    auto& proj_transform = registry.getComponent<transform_component_s>(projectile);
    proj_transform.scale_x = BossDefaults::Projectile::SCALE;
    proj_transform.scale_y = BossDefaults::Projectile::SCALE;

    // Collision avec le joueur
    BoxCollisionComponent collision;
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(projectile, collision);
    registry.addComponent<NetworkIdentity>(projectile, {static_cast<uint32_t>(projectile), 0});

    // Add Lobby Id
    if (lobbyId != 0) {
        registry.addComponent<LobbyIdComponent>(projectile, {lobbyId});
    }
}

// Sous-entités
void BossSystem::updateSubEntities(Registry& registry, system_context context) {
    auto& sub_entities = registry.getEntities<BossSubEntityComponent>();

    for (auto sub_entity : sub_entities) {
        auto& sub = registry.getComponent<BossSubEntityComponent>(sub_entity);

        if (!sub.is_active || sub.is_destroyed)
            continue;

        // Mettre à jour la position relative au boss
        if (registry.hasComponent<transform_component_s>(sub_entity) &&
            registry.hasComponent<transform_component_s>(sub.boss_entity_id)) {
            auto& boss_transform = registry.getConstComponent<transform_component_s>(sub.boss_entity_id);
            auto& sub_transform = registry.getComponent<transform_component_s>(sub_entity);

            sub_transform.x = boss_transform.x + sub.offset_x;
            sub_transform.y = boss_transform.y + sub.offset_y;
        }

        // Tir autonome pour tentacules et canons
        if (sub.type == BossSubEntityComponent::TENTACLE || sub.type == BossSubEntityComponent::CANNON) {
            sub.fire_timer += context.dt;

            if (sub.fire_timer >= sub.fire_rate) {
                sub.fire_timer = 0.0f;
                // Déclencher un tir (à implémenter avec ShooterSystem ou directement ici)

                uint32_t lobbyId = 0;
                if (registry.hasComponent<LobbyIdComponent>(sub.boss_entity_id)) {
                    lobbyId = registry.getComponent<LobbyIdComponent>(sub.boss_entity_id).lobby_id;
                }

                if (registry.hasComponent<transform_component_s>(sub_entity)) {
                    auto& sub_transform = registry.getConstComponent<transform_component_s>(sub_entity);
                    createBossProjectile(registry, context, sub_transform, BossDefaults::SubEntities::PROJECTILE_SPEED,
                                         0.0f, BossDefaults::SubEntities::PROJECTILE_DAMAGE, lobbyId);
                }
            }
        }
    }
}

void BossSystem::spawnTentacle(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y,
                               float fire_rate) {
    Entity tentacle = registry.createEntity();

    BossSubEntityComponent sub;
    sub.boss_entity_id = boss_entity;
    sub.type = BossSubEntityComponent::TENTACLE;
    sub.sub_entity_index = index;
    sub.offset_x = offset_x;
    sub.offset_y = offset_y;
    sub.fire_rate = fire_rate;

    registry.addComponent<BossSubEntityComponent>(tentacle, sub);
    registry.addComponent<HealthComponent>(
        tentacle, {BossDefaults::SubEntities::Tentacle::HP, BossDefaults::SubEntities::Tentacle::HP, 0.0f, 0.5f});
    registry.addComponent<TeamComponent>(tentacle, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(tentacle, {BossDefaults::SubEntities::Tentacle::COLLISION_DAMAGE});

    // Position initiale
    if (registry.hasComponent<transform_component_s>(boss_entity)) {
        auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
        registry.addComponent<transform_component_s>(tentacle,
                                                     {boss_transform.x + offset_x, boss_transform.y + offset_y});
    }

    TagComponent tags;
    tags.tags.push_back("BOSS_PART");
    registry.addComponent<TagComponent>(tentacle, tags);
    registry.addComponent<NetworkIdentity>(tentacle, {static_cast<uint32_t>(tentacle), 0});

    // Add Lobby Id
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        auto& lobby = registry.getComponent<LobbyIdComponent>(boss_entity);
        registry.addComponent<LobbyIdComponent>(tentacle, {lobby.lobby_id});
    }
}

void BossSystem::spawnCannon(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y,
                             float fire_rate) {
    Entity cannon = registry.createEntity();

    BossSubEntityComponent sub;
    sub.boss_entity_id = boss_entity;
    sub.type = BossSubEntityComponent::CANNON;
    sub.sub_entity_index = index;
    sub.offset_x = offset_x;
    sub.offset_y = offset_y;
    sub.fire_rate = fire_rate;

    registry.addComponent<BossSubEntityComponent>(cannon, sub);
    registry.addComponent<HealthComponent>(
        cannon, {BossDefaults::SubEntities::Cannon::HP, BossDefaults::SubEntities::Cannon::HP, 0.0f, 0.5f});
    registry.addComponent<TeamComponent>(cannon, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(cannon, {BossDefaults::SubEntities::Cannon::COLLISION_DAMAGE});

    // Position initiale
    if (registry.hasComponent<transform_component_s>(boss_entity)) {
        auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
        registry.addComponent<transform_component_s>(cannon,
                                                     {boss_transform.x + offset_x, boss_transform.y + offset_y});
    }

    TagComponent tags;
    tags.tags.push_back("BOSS_PART");
    registry.addComponent<TagComponent>(cannon, tags);
    registry.addComponent<NetworkIdentity>(cannon, {static_cast<uint32_t>(cannon), 0});

    // Add Lobby Id
    if (registry.hasComponent<LobbyIdComponent>(boss_entity)) {
        auto& lobby = registry.getComponent<LobbyIdComponent>(boss_entity);
        registry.addComponent<LobbyIdComponent>(cannon, {lobby.lobby_id});
    }
}
