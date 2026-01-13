#include "boss_patterns.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/team_component.hpp"
#include "shooter.hpp"
#include "health.hpp"
#include "damage.hpp"
#include <cmath>
#include <iostream>

void BossPatternSystem::update(Registry& registry, system_context context) {
    auto& bosses = registry.getEntities<BossComponent>();
    
    for (auto boss_entity : bosses) {
        updateBossState(registry, context, boss_entity);
    }
    
    updateSubEntities(registry, context);
}

void BossPatternSystem::updateBossState(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<BossComponent>(boss_entity))
        return;
    
    auto& boss = registry.getComponent<BossComponent>(boss_entity);
    // Update damage flash timer
    if (boss.damage_flash_timer > 0.0f) {
        boss.damage_flash_timer -= context.dt;
        if (boss.damage_flash_timer < 0.0f) {
            boss.damage_flash_timer = 0.0f;
        }
    }
    // Vérifier si le boss a un composant de santé
    if (!registry.hasComponent<HealthComponent>(boss_entity)) {
        return;
    }
    
    auto& health = registry.getComponent<HealthComponent>(boss_entity);
    float health_percent = static_cast<float>(health.current_hp) / static_cast<float>(health.max_hp);

    // Si HP <= 0 et pas déjà en DYING/DEAD, commencer la séquence de mort
    if (health.current_hp <= 0 && boss.current_state != BossComponent::DYING && boss.current_state != BossComponent::DEAD) {
        boss.current_state = BossComponent::DYING;
        boss.death_timer = 0.0f;
        boss.state_timer = 0.0f;
        return;
    }
    
    // Déterminer la phase selon la santé
    BossComponent::BossState target_state = boss.current_state;
    
    if (health_percent <= 0.0f) {
        target_state = BossComponent::DEAD;
    } else if (health_percent <= 0.10f && boss.current_state != BossComponent::ENRAGED) {
        target_state = BossComponent::ENRAGED;
        boss.is_enraged = true;
    } else if (health_percent <= 0.40f && boss.current_state == BossComponent::PHASE_2) {
        target_state = BossComponent::PHASE_3;
    } else if (health_percent <= 0.75f && boss.current_state == BossComponent::PHASE_1) {
        target_state = BossComponent::PHASE_2;
    } else if (boss.has_arrived && boss.current_state == BossComponent::SPAWN) {
        target_state = BossComponent::PHASE_1;
    }
    
    // Transition d'état
    if (target_state != boss.current_state) {
        handlePhaseTransition(registry, boss_entity, boss);
        boss.current_state = target_state;
        boss.state_timer = 0.0f;
    }
    
    boss.state_timer += context.dt;
    
    // Exécuter les patterns selon l'état
    switch (boss.current_state) {
        case BossComponent::SPAWN:
            // Attendre l'arrivée
            break;
            
        case BossComponent::PHASE_1:
            executePhase1Patterns(registry, context, boss_entity, boss);
            break;
            
        case BossComponent::PHASE_2:
            executePhase2Patterns(registry, context, boss_entity, boss);
            break;
            
        case BossComponent::PHASE_3:
            executePhase3Patterns(registry, context, boss_entity, boss);
            break;
            
        case BossComponent::ENRAGED:
            executeEnragedPatterns(registry, context, boss_entity, boss);
            break;
            
        case BossComponent::DYING:
            boss.death_timer += context.dt;
            
            // Déplacer le boss hors écran progressivement pendant la mort pour libérer la vue
            if (registry.hasComponent<transform_component_s>(boss_entity)) {
                auto& transform = registry.getComponent<transform_component_s>(boss_entity);
                // Faire descendre le boss progressivement hors de l'écran
                transform.y += 500.0f * context.dt;
            }
            
            if (boss.death_timer >= boss.death_duration) {
                boss.current_state = BossComponent::DEAD;
                
                // Détruire tous les projectiles ennemis
                auto& projectiles = registry.getEntities<ProjectileComponent>();
                for (auto proj : projectiles) {
                    if (registry.hasComponent<TeamComponent>(proj)) {
                        auto& team = registry.getConstComponent<TeamComponent>(proj);
                        if (team.team == TeamComponent::ENEMY) {
                            if (!registry.hasComponent<PendingDestruction>(proj)) {
                                registry.addComponent<PendingDestruction>(proj, {});
                            }
                        }
                    }
                }
                
                // Détruire toutes les sous-entités du boss
                auto& sub_entities = registry.getEntities<BossSubEntityComponent>();
                for (auto sub : sub_entities) {
                    auto& sub_comp = registry.getConstComponent<BossSubEntityComponent>(sub);
                    if (sub_comp.boss_entity_id == boss_entity) {
                        if (!registry.hasComponent<PendingDestruction>(sub)) {
                            registry.addComponent<PendingDestruction>(sub, {});
                        }
                    }
                }

                // Détruire les segments de queue du boss
                auto& tail_segments = registry.getEntities<BossTailSegmentComponent>();
                for (auto segment : tail_segments) {
                    if (!registry.hasComponent<PendingDestruction>(segment)) {
                        registry.addComponent<PendingDestruction>(segment, {});
                    }
                }

                // IMPORTANT: Use PendingDestruction so the server broadcasts S_ENTITY_DESTROY
                if (!registry.hasComponent<PendingDestruction>(boss_entity)) {
                    registry.addComponent<PendingDestruction>(boss_entity, {});
                }
            }
            break;
            
        case BossComponent::DEAD:
            // Nettoyage final
            break;
    }
}

void BossPatternSystem::handlePhaseTransition(Registry& registry, Entity boss_entity, BossComponent& boss) {
    switch (boss.current_state) {
        case BossComponent::PHASE_1:
            boss.current_phase = 1;
            // Activer 2 tentacules
            spawnTentacle(registry, boss_entity, 0, -100.0f, -80.0f);
            spawnTentacle(registry, boss_entity, 1, -100.0f, 80.0f);
            break;
            
        case BossComponent::PHASE_2:
            boss.current_phase = 2;
            // Activer 4 tentacules + canons
            spawnTentacle(registry, boss_entity, 2, -80.0f, -120.0f);
            spawnTentacle(registry, boss_entity, 3, -80.0f, 120.0f);
            spawnCannon(registry, boss_entity, 0, 0.0f, -150.0f);
            spawnCannon(registry, boss_entity, 1, 0.0f, 150.0f);
            
            // Augmenter fire rate
            if (registry.hasComponent<ShooterComponent>(boss_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(boss_entity);
                shooter.fire_rate = 0.2f;
            }
            break;
            
        case BossComponent::PHASE_3:
            boss.current_phase = 3;
            boss.core_vulnerable = false; // Protection temporaire
            
            if (registry.hasComponent<ShooterComponent>(boss_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(boss_entity);
                shooter.fire_rate = 0.15f;
            }
            break;
            
        case BossComponent::ENRAGED:
            boss.is_enraged = true;
            boss.core_vulnerable = true;
            
            if (registry.hasComponent<ShooterComponent>(boss_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(boss_entity);
                shooter.fire_rate = 0.1f;
            }
            break;
            
        case BossComponent::DYING:
            std::cout << "DYING" << std::endl;
            break;
            
        default:
            break;
    }
}
void BossPatternSystem::executePhase1Patterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss) {
    boss.attack_pattern_timer += context.dt;
    
    if (boss.attack_pattern_timer >= boss.attack_pattern_interval) {
        boss.attack_pattern_timer = 0.0f;
        
        switch (boss.current_attack_pattern % 2) {
            case 0:
                patternLinearAlternate(registry, context, boss_entity);
                break;
            case 1:
                patternSlowMissiles(registry, context, boss_entity);
                break;
        }
        
        boss.current_attack_pattern++;
    }
}
void BossPatternSystem::executePhase2Patterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss) {
    // Oscillation verticale
    boss.oscillation_timer += context.dt;
    if (registry.hasComponent<transform_component_s>(boss_entity)) {
        auto& transform = registry.getComponent<transform_component_s>(boss_entity);
        if (boss.base_y == 0.0f)
            boss.base_y = transform.y;
        transform.y = boss.base_y + std::sin(boss.oscillation_timer * boss.oscillation_frequency) * boss.oscillation_amplitude;
    }
    
    boss.attack_pattern_timer += context.dt;
    
    if (boss.attack_pattern_timer >= boss.attack_pattern_interval) {
        boss.attack_pattern_timer = 0.0f;
        
        switch (boss.current_attack_pattern % 2) {
            case 0:
                patternWallOfProjectiles(registry, context, boss_entity);
                break;
            case 1:
                patternBouncingShots(registry, context, boss_entity);
                break;
        }
        
        boss.current_attack_pattern++;
    }
}
void BossPatternSystem::executePhase3Patterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss) {
    boss.attack_pattern_timer += context.dt;
    
    // Le noyau devient vulnérable après certains patterns
    if (boss.attack_pattern_timer >= 1.0f && boss.attack_pattern_timer < 2.0f) {
        boss.core_vulnerable = true;
    } else {
        boss.core_vulnerable = false;
    }
    
    if (boss.attack_pattern_timer >= boss.attack_pattern_interval) {
        boss.attack_pattern_timer = 0.0f;
        
        switch (boss.current_attack_pattern % 2) {
            case 0:
                patternSpiral(registry, context, boss_entity);
                break;
            case 1:
                patternDelayedShots(registry, context, boss_entity);
                break;
        }
        
        boss.current_attack_pattern++;
    }
}
void BossPatternSystem::executeEnragedPatterns(Registry& registry, system_context context, Entity boss_entity, BossComponent& boss) {
    // Mode ENRAGED ultra-agressif avec mouvements rapides
    boss.oscillation_timer += context.dt * 2.0f; // 2x plus rapide
    if (registry.hasComponent<transform_component_s>(boss_entity)) {
        auto& transform = registry.getComponent<transform_component_s>(boss_entity);
        if (boss.base_y == 0.0f) boss.base_y = transform.y;
        // Oscillation plus ample et rapide
        transform.y = boss.base_y + std::sin(boss.oscillation_timer * 2.0f) * 150.0f;
    }
    
    boss.attack_pattern_timer += context.dt;
    
    // Tirs encore plus rapides en mode ENRAGED
    if (boss.attack_pattern_timer >= 0.8f) {
        boss.attack_pattern_timer = 0.0f;
        
        // Combinaison de patterns pour le chaos
        patternSpiral(registry, context, boss_entity);
        
        // Ajouter un mur de projectiles en alternance
        if (boss.current_attack_pattern % 2 == 0) {
            patternWallOfProjectiles(registry, context, boss_entity);
        }
        
        boss.current_attack_pattern++;
    }
}

// Patterns spécifiques
void BossPatternSystem::patternLinearAlternate(Registry& registry, system_context context, Entity boss_entity) {
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
        
        // Tir linéaire droit vers la gauche
        if (registry.hasComponent<transform_component_s>(sub_entity)) {
            auto& sub_transform = registry.getConstComponent<transform_component_s>(sub_entity);
            createBossProjectile(registry, context, sub_transform, -400.0f, 0.0f, 25);
        }
    }
}

void BossPatternSystem::patternSlowMissiles(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    
    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    
    // 2 missiles lents
    createBossProjectile(registry, context, boss_transform, -250.0f, -100.0f, 30);
    createBossProjectile(registry, context, boss_transform, -250.0f, 100.0f, 30);
}

void BossPatternSystem::patternWallOfProjectiles(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    
    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    
    // Éventail de 5 projectiles
    for (int i = -2; i <= 2; i++) {
        float angle = i * 15.0f; // 15 degrés entre chaque
        float vx = -350.0f * std::cos(angle * 3.14159f / 180.0f);
        float vy = -350.0f * std::sin(angle * 3.14159f / 180.0f);
        createBossProjectile(registry, context, boss_transform, vx, vy, 35);
    }
}

void BossPatternSystem::patternBouncingShots(Registry& registry, system_context context, Entity boss_entity) {
    // TODO: Implémenter rebondissement (nécessite physique avancée)
    // Pour l'instant, tirs diagonaux
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    
    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    
    createBossProjectile(registry, context, boss_transform, -300.0f, -200.0f, 35);
    createBossProjectile(registry, context, boss_transform, -300.0f, 200.0f, 35);
}

void BossPatternSystem::patternSpiral(Registry& registry, system_context context, Entity boss_entity) {
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    if (!registry.hasComponent<BossComponent>(boss_entity))
        return;
    
    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    auto& boss = registry.getConstComponent<BossComponent>(boss_entity);
    
    // Spirale basée sur le timer
    float angle = boss.state_timer * 180.0f; // Rotation rapide
    int projectile_count = 8;
    
    for (int i = 0; i < projectile_count; i++) {
        float current_angle = (angle + (i * 360.0f / projectile_count)) * 3.14159f / 180.0f;
        float vx = -300.0f * std::cos(current_angle);
        float vy = -300.0f * std::sin(current_angle);
        createBossProjectile(registry, context, boss_transform, vx, vy, 40);
    }
}

void BossPatternSystem::patternDelayedShots(Registry& registry, system_context context, Entity boss_entity) {
    // TODO: Implémenter accélération progressive
    // Pour l'instant, tirs lents
    if (!registry.hasComponent<transform_component_s>(boss_entity))
        return;
    
    auto& boss_transform = registry.getConstComponent<transform_component_s>(boss_entity);
    createBossProjectile(registry, context, boss_transform, -200.0f, 0.0f, 45);
}

// Création de projectiles du boss
void BossPatternSystem::createBossProjectile(Registry& registry, system_context context, const transform_component_s& pos, float vx, float vy, int damage) {
    Entity projectile = registry.createEntity();
    
    // Team ennemi
    registry.addComponent<TeamComponent>(projectile, {TeamComponent::ENEMY});
    
    // Tag pour collision avec le joueur
    TagComponent tags;
    tags.tags.push_back("ENEMY_PROJECTILE");
    registry.addComponent<TagComponent>(projectile, tags);
    
    // Marquer comme projectile
    registry.addComponent<ProjectileComponent>(projectile, {projectile});
    
    // Position et vitesse
    registry.addComponent<transform_component_s>(projectile, {pos.x - 30.0f, pos.y});
    registry.addComponent<Velocity2D>(projectile, {vx, vy});
    
    // Dégâts
    registry.addComponent<DamageOnCollision>(projectile, {damage});
    
    // Sprite visible (projectile ennemi rouge)
    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));
    
    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0.0f;
    sprite_info.current_animation_frame = 0;
    // Projectile ennemi (boule d'énergie rouge)
    sprite_info.dimension = {241, 120, 10, 10};
    sprite_info.z_index = 5;
    registry.addComponent<sprite2D_component_s>(projectile, sprite_info);
    
    // Échelle pour rendre visible
    auto& proj_transform = registry.getComponent<transform_component_s>(projectile);
    proj_transform.scale_x = 2.0f;
    proj_transform.scale_y = 2.0f;
    
    // Collision avec le joueur
    BoxCollisionComponent collision;
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(projectile, collision);
}

// Sous-entités
void BossPatternSystem::updateSubEntities(Registry& registry, system_context context) {
    auto& sub_entities = registry.getEntities<BossSubEntityComponent>();
    
    for (auto sub_entity : sub_entities) {
        auto& sub = registry.getComponent<BossSubEntityComponent>(sub_entity);
        
        if (!sub.is_active || sub.is_destroyed)
            continue;
        
        // Mettre à jour la position relative au boss
        if (registry.hasComponent<transform_component_s>(sub_entity) && registry.hasComponent<transform_component_s>(sub.boss_entity_id)) {
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
                // Déclencher un tir (à implémenter avec ShooterSystem)
            }
        }
    }
}

void BossPatternSystem::spawnTentacle(Registry& registry, Entity boss_entity, int index, float offset_x, float offset_y) {
    Entity tentacle = registry.createEntity();
    
    BossSubEntityComponent sub;
    sub.boss_entity_id = boss_entity;
    sub.type = BossSubEntityComponent::TENTACLE;
    sub.sub_entity_index = index;
    sub.offset_x = offset_x;
    sub.offset_y = offset_y;
    sub.fire_rate = 2.0f;
    
    registry.addComponent<BossSubEntityComponent>(tentacle, sub);
    registry.addComponent<HealthComponent>(tentacle, {50, 50, 0.0f, 0.5f});
    registry.addComponent<TeamComponent>(tentacle, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(tentacle, {15});
    
    TagComponent tags;
    tags.tags.push_back("BOSS_PART");
    registry.addComponent<TagComponent>(tentacle, tags);
}

void BossPatternSystem::spawnCannon(Registry& registry, Entity boss_entity, int index,
                                     float offset_x, float offset_y) {
    Entity cannon = registry.createEntity();
    
    BossSubEntityComponent sub;
    sub.boss_entity_id = boss_entity;
    sub.type = BossSubEntityComponent::CANNON;
    sub.sub_entity_index = index;
    sub.offset_x = offset_x;
    sub.offset_y = offset_y;
    sub.fire_rate = 1.5f;
    
    registry.addComponent<BossSubEntityComponent>(cannon, sub);
    registry.addComponent<HealthComponent>(cannon, {80, 80, 0.0f, 0.5f});
    registry.addComponent<TeamComponent>(cannon, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(cannon, {20});
    
    TagComponent tags;
    tags.tags.push_back("BOSS_PART");
    registry.addComponent<TagComponent>(cannon, tags);
}
