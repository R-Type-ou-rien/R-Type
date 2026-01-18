#include "shooter.hpp"

#include <iostream>
#include <iterator>
#include <ostream>
#include <cmath>
#include <algorithm>

#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"
#include "ISystem.hpp"
#include "ResourceConfig.hpp"
#include "damage.hpp"
#include "registry.hpp"
#include "../Components/team_component.hpp"
#include "../Components/charged_shot.hpp"
#include "../Components/pod_component.hpp"
#include "../../../../Engine/Lib/Components/LobbyIdComponent.hpp"
#include "../../../../Engine/Lib/Utils/LobbyUtils.hpp"

Velocity2D ShooterSystem::get_projectile_speed(ShooterComponent::ProjectileType type, TeamComponent::Team team) {
    Velocity2D vel = {0, 0};
    double speed = 0;

    if (team == TeamComponent::ENEMY) {
        speed = 300;  // Slower enemy projectiles (player speed is 350)
    } else {
        switch (type) {
            case ShooterComponent::NORMAL:
                speed = 700;
                break;
            case ShooterComponent::CHARG:
                speed = 800;
                break;
            case ShooterComponent::POD_LASER:
                speed = 800;
                break;
            default:
                speed = 700;
                break;
        }
    }
    vel.vx = speed;
    vel.vy = 0;
    return vel;
}

void ShooterSystem::create_projectile(Registry& registry, Entity owner_entity, ShooterComponent::ProjectileType type,
                                      TeamComponent::Team team, transform_component_s pos, system_context context,
                                      int projectile_damage, float projectile_scale) {
    create_projectile_with_pattern(registry, owner_entity, type, team, pos, context, ShooterComponent::STRAIGHT, 0.0f,
                                   0.0f, projectile_damage, projectile_scale);
}

void ShooterSystem::create_projectile_with_pattern(Registry& registry, Entity owner_entity,
                                                   ShooterComponent::ProjectileType type, TeamComponent::Team team,
                                                   transform_component_s pos, system_context context,
                                                   ShooterComponent::ShootPattern pattern, float target_x,
                                                   float target_y, int projectile_damage, float projectile_scale) {
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

    registry.addComponent<ProjectileComponent>(id, {static_cast<int>(owner_entity)});

    registry.addComponent<TeamComponent>(id, {team});

    float offset_x = (team == TeamComponent::ALLY) ? 50.0f : -20.0f;
    registry.addComponent<transform_component_s>(id, {(pos.x + offset_x), (pos.y + 20)});

    if (projectile_scale > 1.0f) {
        auto& proj_transform = registry.getComponent<transform_component_s>(id);
        proj_transform.scale_x = projectile_scale;
        proj_transform.scale_y = projectile_scale;
    }

    registry.addComponent<Velocity2D>(id, speed);

    registry.addComponent<TagComponent>(id, tags);

    registry.addComponent<DamageOnCollision>(id, {projectile_damage});

    // sprite2D_component_s sprite_info;
    // sprite_info.animation_speed = 0;
    // sprite_info.current_animation_frame = 0;
    // sprite_info.z_index = 5;  // Au-dessus des autres sprites

    // if (team == TeamComponent::ENEMY) {
    //     handle_t<TextureAsset> handle =
    //         context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet30.gif",
    //                                      TextureAsset("src/RType/Common/content/sprites/r-typesheet30.gif"));
    //     sprite_info.handle = handle;
    //     sprite_info.dimension = {200, 230, 12, 12};
    // } else {
    //     handle_t<TextureAsset> handle =
    //         context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
    //                                      TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));
    //     sprite_info.handle = handle;

    //     switch (type) {
    //         case ShooterComponent::NORMAL:
    //         default:
    //             sprite_info.dimension = {232, 103, 32, 14};
    //             break;
    //     }
    // }

    // registry.addComponent<sprite2D_component_s>(id, sprite_info);

    AnimatedSprite2D animation;
    AnimationClip clip;

    clip.frameDuration = 0;
    animation.layer = RenderLayer::Foreground;  // Au-dessus des autres sprites

    if (team == TeamComponent::ENEMY) {
        handle_t<TextureAsset> handle =
            context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet30.gif",
                                         TextureAsset("src/RType/Common/content/sprites/r-typesheet30.gif"));
        clip.handle = handle;
        clip.frames.emplace_back(200, 230, 12, 12);
    } else {
        // Check if owner has ProjectileConfigComponent for custom sprite
        std::string sprite_path = "src/RType/Common/content/sprites/r-typesheet1.gif";
        float sprite_x = 232;
        float sprite_y = 103;
        float sprite_w = 32;
        float sprite_h = 14;

        if (registry.hasComponent<ProjectileConfigComponent>(owner_entity)) {
            const auto& proj_config = registry.getConstComponent<ProjectileConfigComponent>(owner_entity);
            sprite_path = proj_config.projectile_sprite;
            sprite_x = static_cast<float>(proj_config.projectile_sprite_x);
            sprite_y = static_cast<float>(proj_config.projectile_sprite_y);
            sprite_w = static_cast<float>(proj_config.projectile_sprite_w);
            sprite_h = static_cast<float>(proj_config.projectile_sprite_h);
        }

        handle_t<TextureAsset> handle = context.texture_manager.load(sprite_path, TextureAsset(sprite_path));
        clip.handle = handle;
        clip.frames.emplace_back(sprite_x, sprite_y, sprite_w, sprite_h);
    }
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";

    registry.addComponent<AnimatedSprite2D>(id, animation);

    auto& projectile_transform = registry.getComponent<transform_component_s>(id);
    if (team == TeamComponent::ENEMY) {
        projectile_transform.scale_x = 4.0f;
        projectile_transform.scale_y = 4.0f;
    } else {
        projectile_transform.scale_x = 2.0f;
        projectile_transform.scale_y = 2.0f;
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

    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});

    // Inherit LobbyIdComponent from owner
    uint32_t owner_lobby_id = engine::utils::getLobbyId(registry, owner_entity);
    if (owner_lobby_id != 0) {
        registry.addComponent<LobbyIdComponent>(id, {owner_lobby_id});
    }

    return;
}

void ShooterSystem::create_charged_projectile(Registry& registry, Entity owner_entity, TeamComponent::Team team,
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

    registry.addComponent<ProjectileComponent>(id, {static_cast<int>(owner_entity)});
    registry.addComponent<TeamComponent>(id, {team});
    registry.addComponent<transform_component_s>(id, {(pos.x + 50), (pos.y)});
    auto& proj_transform = registry.getComponent<transform_component_s>(id);
    if (charge_ratio >= 1.0f) {
        proj_transform.scale_x = 2.0f;
        proj_transform.scale_y = 2.0f;
    } else if (charge_ratio >= 0.5f) {
        proj_transform.scale_x = 1.5f;
        proj_transform.scale_y = 1.5f;
    } else {
        proj_transform.scale_x = 1.0f;
        proj_transform.scale_y = 1.0f;
    }
    registry.addComponent<Velocity2D>(id, speed);
    registry.addComponent<TagComponent>(id, tags);

    int damage = static_cast<int>(50 + (150 * charge_ratio));
    registry.addComponent<DamageOnCollision>(id, {damage});

    if (charge_ratio >= 1.0f) {
        registry.addComponent<PenetratingProjectile>(id, {999, 0});
    } else if (charge_ratio >= 0.5f) {
        registry.addComponent<PenetratingProjectile>(id, {3, 0});
    }

    // Get charged projectile sprite from config or use defaults
    std::string sprite_path = "src/RType/Common/content/sprites/r-typesheet1.gif";
    float sprite_x = 263, sprite_y = 120;
    float base_w = 64, base_h = 56;

    if (registry.hasComponent<ProjectileConfigComponent>(owner_entity)) {
        const auto& proj_config = registry.getConstComponent<ProjectileConfigComponent>(owner_entity);
        sprite_path = proj_config.charged_sprite;
        sprite_x = static_cast<float>(proj_config.charged_sprite_x);
        sprite_y = static_cast<float>(proj_config.charged_sprite_y);
        base_w = static_cast<float>(proj_config.charged_sprite_w);
        base_h = static_cast<float>(proj_config.charged_sprite_h);
    }

    handle_t<TextureAsset> handle = context.texture_manager.load(sprite_path, TextureAsset(sprite_path));

    AnimatedSprite2D animation;
    AnimationClip clip;

    clip.handle = handle;
    clip.frameDuration = 0.1f;
    clip.mode = AnimationMode::Loop;

    // 4 animation frames for charged projectile
    if (charge_ratio >= 0.8f) {
        clip.frames.emplace_back(sprite_x, sprite_y, base_w, base_h);
        clip.frames.emplace_back(sprite_x + base_w, sprite_y, base_w, base_h);
        clip.frames.emplace_back(sprite_x + base_w * 2, sprite_y, base_w, base_h);
        clip.frames.emplace_back(sprite_x + base_w * 3, sprite_y, base_w, base_h);
    } else if (charge_ratio >= 0.5f) {
        float w = base_w * 0.75f;
        float h = base_h * 0.75f;
        clip.frames.emplace_back(sprite_x, sprite_y, w, h);
        clip.frames.emplace_back(sprite_x + base_w, sprite_y, w, h);
        clip.frames.emplace_back(sprite_x + base_w * 2, sprite_y, w, h);
        clip.frames.emplace_back(sprite_x + base_w * 3, sprite_y, w, h);
    } else {
        float w = base_w * 0.5f;
        float h = base_h * 0.5f;
        clip.frames.emplace_back(sprite_x, sprite_y, w, h);
        clip.frames.emplace_back(sprite_x + base_w, sprite_y, w, h);
        clip.frames.emplace_back(sprite_x + base_w * 2, sprite_y, w, h);
        clip.frames.emplace_back(sprite_x + base_w * 3, sprite_y, w, h);
    }
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";

    registry.addComponent<AnimatedSprite2D>(id, animation);

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

    // Add NetworkIdentity for network replication
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});

    // Inherit LobbyIdComponent from owner
    uint32_t owner_lobby_id = engine::utils::getLobbyId(registry, owner_entity);
    if (owner_lobby_id != 0) {
        registry.addComponent<LobbyIdComponent>(id, {owner_lobby_id});
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

                // Play charging sound when reaching medium threshold (50%)
                if (charged.charge_time >= charged.medium_charge && !registry.hasComponent<AudioSourceComponent>(id)) {
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

                if (shooter.last_shot >= shooter.fire_rate) {
                    int proj_damage = shooter.projectile_damage;
                    // 3 shot types based on charge level:
                    // 1. Normal shot: charge < 50% (< 1.0s)
                    // 2. Medium charged shot: charge >= 50% (>= 1.0s, yellow bar)
                    // 3. Max charged shot: charge >= 100% (>= 2.0s, full red bar)
                    if (charged.charge_time >= charged.max_charge_time) {
                        // Max charged shot (100%) - full power
                        create_charged_projectile(registry, id, team.team, pos, context, 1.0f);
                    } else if (charged.charge_time >= charged.medium_charge) {
                        // Medium charged shot (50%) - half power
                        create_charged_projectile(registry, id, team.team, pos, context, 0.5f);
                    } else {
                        // Normal shot - no charge
                        create_projectile(registry, id, shooter.type, team.team, pos, context, proj_damage,
                                          shooter.projectile_scale);
                    }
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

        // Update pod laser cooldown
        if (shooter.use_pod_laser && team.team == TeamComponent::ALLY) {
            if (registry.hasComponent<PlayerPodComponent>(id)) {
                auto& player_pod = registry.getComponent<PlayerPodComponent>(id);
                if (player_pod.pod_laser_cooldown > 0.0f) {
                    player_pod.pod_laser_cooldown -= context.dt;
                }
            }
        }

        if (shooter.last_shot >= shooter.fire_rate) {
            int proj_damage = shooter.projectile_damage;

            if (shooter.use_pod_laser && team.team == TeamComponent::ALLY) {
                if (registry.hasComponent<PlayerPodComponent>(id)) {
                    auto& player_pod = registry.getComponent<PlayerPodComponent>(id);
                    // Only fire if pod laser cooldown has expired
                    if (player_pod.has_pod && player_pod.pod_attached && player_pod.pod_entity != -1 &&
                        player_pod.pod_laser_cooldown <= 0.0f) {
                        if (registry.hasComponent<transform_component_s>(player_pod.pod_entity)) {
                            auto& pod_pos = registry.getConstComponent<transform_component_s>(player_pod.pod_entity);
                            create_pod_circular_laser(registry, id, pod_pos, context, proj_damage);
                            player_pod.pod_laser_cooldown = player_pod.pod_laser_fire_rate;
                        }
                    }
                }
            } else if (shooter.pattern == ShooterComponent::SPREAD && team.team == TeamComponent::ENEMY) {
                create_projectile_with_pattern(registry, id, shooter.type, team.team, pos, context,
                                               ShooterComponent::STRAIGHT, 0, 0, proj_damage, shooter.projectile_scale);

                transform_component_s pos_up = pos;
                pos_up.y -= 20;
                create_projectile_with_pattern(registry, id, shooter.type, team.team, pos_up, context,
                                               ShooterComponent::STRAIGHT, 0, -200, proj_damage,
                                               shooter.projectile_scale);

                transform_component_s pos_down = pos;
                pos_down.y += 20;
                create_projectile_with_pattern(registry, id, shooter.type, team.team, pos_down, context,
                                               ShooterComponent::STRAIGHT, 0, 200, proj_damage,
                                               shooter.projectile_scale);
            } else if (shooter.pattern == ShooterComponent::AIM_PLAYER && player_entity != -1) {
                create_projectile_with_pattern(registry, id, shooter.type, team.team, pos, context,
                                               ShooterComponent::AIM_PLAYER, player_x, player_y, proj_damage,
                                               shooter.projectile_scale);
            } else {
                create_projectile_with_pattern(registry, id, shooter.type, team.team, pos, context,
                                               ShooterComponent::STRAIGHT, 0, 0, proj_damage, shooter.projectile_scale);
            }
            shooter.last_shot = 0.f;
        }
        if (team.team == TeamComponent::ALLY) {
            shooter.is_shooting = false;
        }
    }
}

void ShooterSystem::create_pod_circular_laser(Registry& registry, Entity owner_entity, transform_component_s pos,
                                              system_context context, int projectile_damage) {
    Entity laser_id = registry.createEntity();

    Velocity2D speed = {800.0f, 0.0f};

    registry.addComponent<transform_component_s>(laser_id, {pos.x + 30.0f, pos.y - 10.0f, 4.0f, 4.0f});
    registry.addComponent<Velocity2D>(laser_id, speed);

    TagComponent tags;
    tags.tags.push_back("FRIENDLY_PROJECTILE");
    tags.tags.push_back("POD_LASER");
    registry.addComponent<TagComponent>(laser_id, tags);
    registry.addComponent<TeamComponent>(laser_id, {TeamComponent::ALLY});
    registry.addComponent<ProjectileComponent>(laser_id, {static_cast<int>(owner_entity)});
    registry.addComponent<DamageOnCollision>(laser_id, {projectile_damage});
    registry.addComponent<PenetratingProjectile>(laser_id, {999, 0});

    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/bolt.png",
                                     TextureAsset("src/RType/Common/content/sprites/bolt.png"));

    AnimatedSprite2D animation;
    AnimationClip clip;

    clip.handle = handle;
    clip.frameDuration = 0.1f;
    clip.mode = AnimationMode::Loop;

    clip.frames.emplace_back(0, 0, 48, 32);
    clip.frames.emplace_back(48, 0, 48, 32);
    clip.frames.emplace_back(96, 0, 48, 32);
    clip.frames.emplace_back(144, 0, 48, 32);

    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";

    registry.addComponent<AnimatedSprite2D>(laser_id, animation);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("AI");
    registry.addComponent<BoxCollisionComponent>(laser_id, collision);

    registry.addComponent<NetworkIdentity>(laser_id, {static_cast<uint32_t>(laser_id), 0});

    // Inherit LobbyIdComponent from owner
    uint32_t owner_lobby_id = engine::utils::getLobbyId(registry, owner_entity);
    if (owner_lobby_id != 0) {
        registry.addComponent<LobbyIdComponent>(laser_id, {owner_lobby_id});
    }
}
