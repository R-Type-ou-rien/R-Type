#include "shooter.hpp"

#include <iostream>
#include <iterator>
#include <ostream>
#include <cmath>

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "ResourceConfig.hpp"
#include "damage.hpp"
#include "registry.hpp"
#include "../Components/team_component.hpp"
#include "../Components/charged_shot.hpp"

Velocity2D ShooterSystem::get_projectile_speed(ShooterComponent::ProjectileType type, TeamComponent::Team team) {
    Velocity2D vel = {0, 0};
    double speed = 0;

    switch (type) {
        case ShooterComponent::NORMAL:
            speed = 700;
            break;
        case ShooterComponent::CHARG:
            speed = 800;
            break;
        case ShooterComponent::RED:
            speed = 650;
            break;
        case ShooterComponent::BLUE:
            speed = 650;
            break;
    }
    vel.vx = speed;
    vel.vy = 0;
    return vel;
}

void ShooterSystem::create_projectile(Registry& registry, ShooterComponent::ProjectileType type,
                                      TeamComponent::Team team, transform_component_s pos, system_context context) {
    int id = registry.createEntity();
    Velocity2D speed = get_projectile_speed(type, team);

    if (team == TeamComponent::ENEMY) {
        speed.vx = -speed.vx;
    }

    TagComponent tags;
    if (team == TeamComponent::ALLY) {
        tags.tags.push_back("FRIENDLY_PROJECTILE");
    } else {
        tags.tags.push_back("ENEMY_PROJECTILE");
    }

    registry.addComponent<ProjectileComponent>(id, {id});

    registry.addComponent<TeamComponent>(id, {team});

    // Position ajustée selon l'équipe
    float offset_x = (team == TeamComponent::ALLY) ? 50.0f : -20.0f;
    registry.addComponent<transform_component_s>(id, {(pos.x + offset_x), (pos.y + 20)});

    registry.addComponent<Velocity2D>(id, speed);

    registry.addComponent<TagComponent>(id, tags);

    registry.addComponent<DamageOnCollision>(id, {30});

    handle_t<TextureAsset> handle = context.texture_manager.load("content/sprites/r-typesheet1.gif",
                                                                 TextureAsset("content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    // Projectile plus visible (32x14 au lieu de 17x13)
    sprite_info.dimension = {232, 103, 32, 14};
    sprite_info.z_index = 1;

    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    registry.addComponent<BoxCollisionComponent>(id, {});
    BoxCollisionComponent& collision = registry.getComponent<BoxCollisionComponent>(id);
    if (team == TeamComponent::ALLY) {
        collision.tagCollision.push_back("AI");
    } else {
        collision.tagCollision.push_back("PLAYER");
    }
    return;
}

void ShooterSystem::create_charged_projectile(Registry& registry, TeamComponent::Team team,
                                             transform_component_s pos, system_context context, float charge_ratio) {
    int id = registry.createEntity();
    
    Velocity2D speed = {700, 0};
    if (team == TeamComponent::ENEMY) {
        speed.vx = -speed.vx;
    }

    TagComponent tags;
    if (team == TeamComponent::ALLY) {
        tags.tags.push_back("FRIENDLY_PROJECTILE");
    } else {
        tags.tags.push_back("ENEMY_PROJECTILE");
    }

    registry.addComponent<ProjectileComponent>(id, {id});
    registry.addComponent<TeamComponent>(id, {team});
    registry.addComponent<transform_component_s>(id, {(pos.x + 50), (pos.y)});
    registry.addComponent<Velocity2D>(id, speed);
    registry.addComponent<TagComponent>(id, tags);
    
    int damage = static_cast<int>(50 + (150 * charge_ratio));
    registry.addComponent<DamageOnCollision>(id, {damage});
    
    registry.addComponent<PenetratingProjectile>(id, {});

    // Utiliser r-typesheet1.gif qui a les projectiles (zone avec énergie)
    handle_t<TextureAsset> handle = context.texture_manager.load("content/sprites/r-typesheet1.gif",
                                                                TextureAsset("content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    // Sprite de projectile d'énergie (plus gros selon la charge)
    if (charge_ratio >= 0.8f) {
        sprite_info.dimension = {263, 120, 64, 56};  // Gros tir chargé
    } else if (charge_ratio >= 0.5f) {
        sprite_info.dimension = {263, 120, 48, 42};  // Tir moyen
    } else {
        sprite_info.dimension = {263, 120, 32, 28};  // Petit tir chargé
    }
    sprite_info.z_index = 2;

    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    registry.addComponent<BoxCollisionComponent>(id, {});
    BoxCollisionComponent& collision = registry.getComponent<BoxCollisionComponent>(id);
    if (team == TeamComponent::ALLY) {
        collision.tagCollision.push_back("AI");
    } else {
        collision.tagCollision.push_back("PLAYER");
    }
}

void ShooterSystem::update(Registry& registry, system_context context) {
    auto& shootersIds = registry.getEntities<ShooterComponent>();

    for (auto id : shootersIds) {
        if (!registry.hasComponent<transform_component_s>(id)) {
            continue;
        }
        if (!registry.hasComponent<TeamComponent>(id)) {
            continue;
        }
        ShooterComponent& shooter = registry.getComponent<ShooterComponent>(id);
        shooter.last_shot += context.dt;
        
        if (registry.hasComponent<ChargedShotComponent>(id)) {
            ChargedShotComponent& charged = registry.getComponent<ChargedShotComponent>(id);
            
            if (shooter.trigger_pressed && shooter.is_shooting) {
                charged.is_charging = true;
                charged.charge_time += context.dt;
                if (charged.charge_time > charged.max_charge_time) {
                    charged.charge_time = charged.max_charge_time;
                }
                continue;
            }
            
            if (!shooter.trigger_pressed && charged.is_charging) {
                const transform_component_s& pos = registry.getConstComponent<transform_component_s>(id);
                const TeamComponent& team = registry.getConstComponent<TeamComponent>(id);
                
                if (charged.charge_time >= charged.min_charge_time && shooter.last_shot >= shooter.fire_rate) {
                    float charge_ratio = (charged.charge_time - charged.min_charge_time) / 
                                        (charged.max_charge_time - charged.min_charge_time);
                    charge_ratio = std::min(1.0f, charge_ratio);
                    create_charged_projectile(registry, team.team, pos, context, charge_ratio);
                    shooter.last_shot = 0.f;
                } 
                else if (charged.charge_time < charged.min_charge_time && shooter.last_shot >= shooter.fire_rate) {
                    create_projectile(registry, shooter.type, team.team, pos, context);
                    shooter.last_shot = 0.f;
                }
                
                charged.is_charging = false;
                charged.charge_time = 0.0f;
                shooter.is_shooting = false;
                continue;
            }
        }
        
        if (!shooter.is_shooting)
            continue;
        const transform_component_s& pos = registry.getConstComponent<transform_component_s>(id);
        const TeamComponent& team = registry.getConstComponent<TeamComponent>(id);

        if (shooter.last_shot >= shooter.fire_rate) {
            create_projectile(registry, shooter.type, team.team, pos, context);
            shooter.last_shot = 0.f;
        }
        shooter.is_shooting = false;
    }
}
