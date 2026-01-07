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
#include "team_component.hpp"
#include "charged_shot.hpp"
#include "Components/AudioComponent.hpp"

Velocity2D ShooterSystem::get_projectile_speed(ShooterComponent::ProjectileType type, TeamComponent::Team team) {
    Velocity2D vel = {0, 0};
    double speed = 0;

    switch (type) {
        case ShooterComponent::NORMAL:
            speed = 500;
            break;
        case ShooterComponent::CHARG:
            speed = 7;
            break;
        case ShooterComponent::RED:
            speed = 10;
            break;
        case ShooterComponent::BLUE:
            speed = 10;
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
        tags.tags.push_back("ENEMY_PROJECTILE");
    } else {
        tags.tags.push_back("PLAYER_PROJECTILE");
    }

    registry.addComponent<ProjectileComponent>(id, {id});

    registry.addComponent<TeamComponent>(id, {team});

    registry.addComponent<transform_component_s>(id, {(pos.x + 32), (pos.y + 8)});

    registry.addComponent<Velocity2D>(id, speed);

    registry.addComponent<TagComponent>(id, tags);

    registry.addComponent<DamageOnCollision>(id, {10});

    handle_t<TextureAsset> handle = context.texture_manager.load("content/sprites/r-typesheet1.gif",
                                                                 TextureAsset("content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    sprite_info.dimension = {230, 103, 17, 13};
    sprite_info.z_index = 1;

    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    registry.addComponent<BoxCollisionComponent>(id, {});
    BoxCollisionComponent& collision = registry.getComponent<BoxCollisionComponent>(id);
    if (team == TeamComponent::ALLY) {
        collision.tagCollision.push_back("AI");
    } else {
        collision.tagCollision.push_back("PLAYER");
    }

#if defined(CLIENT_BUILD)
    int soundEntity = registry.createEntity();
    AudioSourceComponent audio;
    audio.sound_name = "shoot";
    audio.play_on_start = true;
    registry.addComponent<AudioSourceComponent>(soundEntity, audio);
#endif

    return;
}

void ShooterSystem::create_charged_projectile(Registry& registry, TeamComponent::Team team, transform_component_s pos,
                                              system_context context, float charge_ratio) {
    int id = registry.createEntity();

    Velocity2D speed = {600, 0};
    if (team == TeamComponent::ENEMY) {
        speed.vx = -speed.vx;
    }

    TagComponent tags;
    if (team == TeamComponent::ALLY) {
        tags.tags.push_back("ENEMY_PROJECTILE");
    } else {
        tags.tags.push_back("PLAYER_PROJECTILE");
    }

    registry.addComponent<ProjectileComponent>(id, {id});
    registry.addComponent<TeamComponent>(id, {team});
    registry.addComponent<transform_component_s>(id, {(pos.x + 32), (pos.y + 8)});
    registry.addComponent<Velocity2D>(id, speed);
    registry.addComponent<TagComponent>(id, tags);

    int damage = static_cast<int>(30 + (70 * charge_ratio));
    registry.addComponent<DamageOnCollision>(id, {damage});

    registry.addComponent<PenetratingProjectile>(id, {});

    handle_t<TextureAsset> handle = context.texture_manager.load("content/sprites/r-typesheet1.gif",
                                                                 TextureAsset("content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    if (charge_ratio >= 0.8f) {
        sprite_info.dimension = {232, 120, 31, 13};
    } else if (charge_ratio >= 0.5f) {
        sprite_info.dimension = {265, 120, 32, 14};
    } else {
        sprite_info.dimension = {200, 120, 30, 12};
    }
    sprite_info.z_index = 1;

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

#if defined(CLIENT_BUILD)
                if (charged.charging_sound_entity == -1) {
                    int soundEntity = registry.createEntity();
                    AudioSourceComponent audio;
                    audio.sound_name = "charg_start";
                    audio.play_on_start = true;
                    audio.loop = false;
                    audio.next_sound_name = "charg_loop";
                    audio.next_sound_loop = true;

                    registry.addComponent<AudioSourceComponent>(soundEntity, audio);
                    charged.charging_sound_entity = soundEntity;
                }
#endif

                charged.charge_time += context.dt;
                if (charged.charge_time > charged.max_charge_time) {
                    charged.charge_time = charged.max_charge_time;
                }
                continue;
            }

            if (!shooter.trigger_pressed && charged.is_charging) {
                const transform_component_s& pos = registry.getConstComponent<transform_component_s>(id);
                const TeamComponent& team = registry.getConstComponent<TeamComponent>(id);

#if defined(CLIENT_BUILD)
                if (charged.charging_sound_entity != -1) {
                    if (registry.hasComponent<AudioSourceComponent>(charged.charging_sound_entity)) {
                        auto& audio = registry.getComponent<AudioSourceComponent>(charged.charging_sound_entity);
                        audio.stop_requested = true;
                    }
                    charged.charging_sound_entity = -1;
                }
#endif

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
            create_projectile(registry, shooter.type, team.team, pos, context);
            shooter.last_shot = 0.f;
        }
        shooter.is_shooting = false;
    }
}
