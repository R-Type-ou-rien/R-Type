#include "shooter.hpp"

#include <iostream>
#include <iterator>
#include <ostream>
#include <cmath>
#include <algorithm>

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "ResourceConfig.hpp"
#include "damage.hpp"
#include "registry.hpp"
#include "../Components/team_component.hpp"
#include "../Components/charged_shot.hpp"
#include "../Components/pod_component.hpp"

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
    create_projectile_with_pattern(registry, type, team, pos, context, ShooterComponent::STRAIGHT, 0.0f, 0.0f, 30);
}

void ShooterSystem::create_projectile_with_pattern(Registry& registry, ShooterComponent::ProjectileType type,
                                                   TeamComponent::Team team, transform_component_s pos,
                                                   system_context context, ShooterComponent::ShootPattern pattern,
                                                   float target_x, float target_y, int projectile_damage) {
    int id = registry.createEntity();
    Velocity2D speed = get_projectile_speed(type, team);

    if (pattern == ShooterComponent::AIM_PLAYER && team == TeamComponent::ENEMY) {
        float dx = target_x - pos.x;
        float dy = target_y - pos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0) {
            float projectile_speed = std::sqrt(speed.vx * speed.vx + speed.vy * speed.vy);
            speed.vx = (dx / distance) * projectile_speed;
            speed.vy = (dy / distance) * projectile_speed;
        }
    } else if (team == TeamComponent::ENEMY) {
        speed.vx = -std::abs(speed.vx);
        speed.vy = 0;
    }

    TagComponent tags;
    if (team == TeamComponent::ALLY) {
        tags.tags.push_back("FRIENDLY_PROJECTILE");
    } else {
        tags.tags.push_back("ENEMY_PROJECTILE");
    }

    registry.addComponent<ProjectileComponent>(id, {id});

    registry.addComponent<TeamComponent>(id, {team});

    float offset_x = (team == TeamComponent::ALLY) ? 50.0f : -20.0f;
    registry.addComponent<transform_component_s>(id, {(pos.x + offset_x), (pos.y + 20)});

    registry.addComponent<Velocity2D>(id, speed);

    registry.addComponent<TagComponent>(id, tags);

    registry.addComponent<DamageOnCollision>(id, {projectile_damage});

    sprite2D_component_s sprite_info;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    sprite_info.z_index = 5;  // Au-dessus des autres sprites

    // Différencier les sprites selon l'équipe
    if (team == TeamComponent::ENEMY) {
        // Projectiles ennemis : petites boules rouges du boss (r-typesheet30.gif)
        handle_t<TextureAsset> handle =
            context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet30.gif",
                                         TextureAsset("src/RType/Common/content/sprites/r-typesheet30.gif"));
        sprite_info.handle = handle;
        // Coordonnées des petites boules rouges/orange en bas du sprite du boss
        sprite_info.dimension = {200, 230, 12, 12};  // Petite boule rouge
    } else {
        // Projectiles alliés : projectiles verts du joueur (r-typesheet1.gif)
        handle_t<TextureAsset> handle =
            context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
                                         TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));
        sprite_info.handle = handle;
        // Projectile plus visible (32x14 au lieu de 17x13)
        sprite_info.dimension = {232, 103, 32, 14};
    }

    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    auto& projectile_transform = registry.getComponent<transform_component_s>(id);
    if (team == TeamComponent::ENEMY) {
        // Boules rouges ennemies : agrandir pour visibilité (pas de flip car c'est une boule)
        projectile_transform.scale_x = 3.5f;  // Agrandir davantage pour être bien visible
        projectile_transform.scale_y = 3.5f;  // Agrandir davantage pour être bien visible
    } else {
        projectile_transform.scale_x = 1.5f;
        projectile_transform.scale_y = 1.5f;
    }

    registry.addComponent<BoxCollisionComponent>(id, {});
    BoxCollisionComponent& collision = registry.getComponent<BoxCollisionComponent>(id);
    if (team == TeamComponent::ALLY) {
        collision.tagCollision.push_back("AI");
    } else {
        collision.tagCollision.push_back("PLAYER");
    }

    if (team == TeamComponent::ALLY) {
        AudioSourceComponent audio;
        audio.sound_name = "shoot";
        audio.play_on_start = true;
        audio.loop = false;
        audio.destroy_entity_on_finish = false;
        registry.addComponent<AudioSourceComponent>(id, audio);
    }

    return;
}

void ShooterSystem::create_charged_projectile(Registry& registry, TeamComponent::Team team, transform_component_s pos,
                                              system_context context, float charge_ratio) {
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

    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    if (charge_ratio >= 0.8f) {
        sprite_info.dimension = {263, 120, 64, 56};
    } else if (charge_ratio >= 0.5f) {
        sprite_info.dimension = {263, 120, 48, 42};
    } else {
        sprite_info.dimension = {263, 120, 32, 28};
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

    if (team == TeamComponent::ALLY) {
        AudioSourceComponent audio;
        audio.sound_name = "shoot";
        audio.play_on_start = true;
        audio.loop = false;
        audio.destroy_entity_on_finish = false;
        registry.addComponent<AudioSourceComponent>(id, audio);
    }
}

void ShooterSystem::update(Registry& registry, system_context context) {
    auto& shootersIds = registry.getEntities<ShooterComponent>();

    Entity player_entity = -1;
    float player_x = 0.0f, player_y = 0.0f;
    auto& teams = registry.getEntities<TeamComponent>();
    for (auto entity : teams) {
        auto& team = registry.getConstComponent<TeamComponent>(entity);
        if (team.team == TeamComponent::ALLY) {
            if (registry.hasComponent<TagComponent>(entity)) {
                auto& tags = registry.getConstComponent<TagComponent>(entity);
                for (const auto& tag : tags.tags) {
                    if (tag == "PLAYER") {
                        player_entity = entity;
                        if (registry.hasComponent<transform_component_s>(entity)) {
                            auto& player_pos = registry.getConstComponent<transform_component_s>(entity);
                            player_x = player_pos.x;
                            player_y = player_pos.y;
                        }
                        break;
                    }
                }
            }
        }
        if (player_entity != -1)
            break;
    }

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

                if (charged.charge_time >= charged.min_charge_time &&
                    !registry.hasComponent<AudioSourceComponent>(id)) {
                    AudioSourceComponent audio;
                    audio.sound_name = "charg_start";
                    audio.play_on_start = true;
                    audio.loop = false;
                    audio.next_sound_name = "charg_loop";
                    audio.next_sound_loop = true;
                    audio.destroy_entity_on_finish = false;
                    registry.addComponent<AudioSourceComponent>(id, audio);
                }

                if (charged.charge_time > charged.max_charge_time) {
                    charged.charge_time = charged.max_charge_time;
                }
                continue;
            }

            if (!shooter.trigger_pressed && charged.is_charging) {
                if (registry.hasComponent<AudioSourceComponent>(id)) {
                    auto& audio = registry.getComponent<AudioSourceComponent>(id);
                    audio.stop_requested = true;
                }

                const transform_component_s& pos = registry.getConstComponent<transform_component_s>(id);
                const TeamComponent& team = registry.getConstComponent<TeamComponent>(id);

                if (charged.charge_time >= charged.min_charge_time && shooter.last_shot >= shooter.fire_rate) {
                    float charge_ratio = (charged.charge_time - charged.min_charge_time) /
                                         (charged.max_charge_time - charged.min_charge_time);
                    charge_ratio = std::min(1.0f, charge_ratio);
                    create_charged_projectile(registry, team.team, pos, context, charge_ratio);
                    shooter.last_shot = 0.f;
                } else if (charged.charge_time < charged.min_charge_time && shooter.last_shot >= shooter.fire_rate) {
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
            int proj_damage = shooter.projectile_damage;

            if (shooter.use_pod_laser && team.team == TeamComponent::ALLY) {
                if (registry.hasComponent<PlayerPodComponent>(id)) {
                    auto& player_pod = registry.getConstComponent<PlayerPodComponent>(id);
                    if (player_pod.has_pod && player_pod.pod_attached && player_pod.pod_entity != -1) {
                        if (registry.hasComponent<transform_component_s>(player_pod.pod_entity)) {
                            auto& pod_pos = registry.getConstComponent<transform_component_s>(player_pod.pod_entity);
                            create_pod_circular_laser(registry, pod_pos, context, proj_damage);
                        }
                    }
                }
            } else if (shooter.pattern == ShooterComponent::SPREAD && team.team == TeamComponent::ENEMY) {
                create_projectile_with_pattern(registry, shooter.type, team.team, pos, context,
                                               ShooterComponent::STRAIGHT, 0, 0, proj_damage);

                transform_component_s pos_up = pos;
                pos_up.y -= 20;
                create_projectile_with_pattern(registry, shooter.type, team.team, pos_up, context,
                                               ShooterComponent::STRAIGHT, 0, -200, proj_damage);

                transform_component_s pos_down = pos;
                pos_down.y += 20;
                create_projectile_with_pattern(registry, shooter.type, team.team, pos_down, context,
                                               ShooterComponent::STRAIGHT, 0, 200, proj_damage);
            } else if (shooter.pattern == ShooterComponent::AIM_PLAYER && player_entity != -1) {
                create_projectile_with_pattern(registry, shooter.type, team.team, pos, context,
                                               ShooterComponent::AIM_PLAYER, player_x, player_y, proj_damage);
            } else {
                create_projectile_with_pattern(registry, shooter.type, team.team, pos, context,
                                               ShooterComponent::STRAIGHT, 0, 0, proj_damage);
            }
            shooter.last_shot = 0.f;
        }
        if (team.team == TeamComponent::ALLY) {
            shooter.is_shooting = false;
        }
    }
}

void ShooterSystem::create_pod_circular_laser(Registry& registry, transform_component_s pos, system_context context,
                                              int projectile_damage) {
    Entity laser_id = registry.createEntity();

    Velocity2D speed = {800.0f, 0.0f};

    registry.addComponent<transform_component_s>(laser_id, {pos.x + 30.0f, pos.y, 3.0f, 2.0f});
    registry.addComponent<Velocity2D>(laser_id, speed);

    TagComponent tags;
    tags.tags.push_back("FRIENDLY_PROJECTILE");
    tags.tags.push_back("POD_LASER");
    registry.addComponent<TagComponent>(laser_id, tags);
    registry.addComponent<TeamComponent>(laser_id, {TeamComponent::ALLY});
    registry.addComponent<ProjectileComponent>(laser_id, {static_cast<int>(laser_id)});
    registry.addComponent<DamageOnCollision>(laser_id, {projectile_damage});
    registry.addComponent<PenetratingProjectile>(laser_id, {999, 0});

    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    sprite_info.dimension = {263, 120, 80, 48};
    sprite_info.z_index = 3;

    registry.addComponent<sprite2D_component_s>(laser_id, sprite_info);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("AI");
    registry.addComponent<BoxCollisionComponent>(laser_id, collision);

    Entity sound_entity = registry.createEntity();
    AudioSourceComponent audio;
    audio.sound_name = "shoot";  // remplacer par un son de laser wesh
    audio.play_on_start = true;
    audio.loop = false;
    audio.destroy_entity_on_finish = true;
    registry.addComponent<AudioSourceComponent>(sound_entity, audio);
}
