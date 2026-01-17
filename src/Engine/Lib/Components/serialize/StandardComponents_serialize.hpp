#pragma once

#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "serialize.hpp"
#include "../../../../RType/Common/Components/damage_component.hpp"
#include "../StructDatas/Rect2D.hpp"
#include "../Components/Sprite/AnimatedSprite2D.hpp"
#include "../../../../RType/Common/Components/shooter_component.hpp"
#include "../../../../RType/Common/Components/team_component.hpp"
#include "../../../../RType/Common/Components/game_timer.hpp"
#include "../../../../RType/Common/Components/charged_shot.hpp"
#include "../../../../RType/Common/Components/spawn.hpp"
#include "../../../../RType/Common/Components/pod_component.hpp"
#include "../../../../RType/Common/Components/behavior_component.hpp"
#include "../../../../RType/Common/Components/boss_component.hpp"
#include "../../../../RType/Common/Systems/health.hpp"
#include "Components/NetworkComponents.hpp"
#include "Components/AudioComponent.hpp"

namespace serialize {

/** AudioSource Component */
inline void serialize(std::vector<uint8_t>& buffer, const AudioSourceComponent& component) {
    serialize(buffer, component.sound_name);
    serialize(buffer, component.play_on_start);
    serialize(buffer, component.loop);
    serialize(buffer, component.destroy_entity_on_finish);
}

inline AudioSourceComponent deserialize_audio_source(const std::vector<uint8_t>& buffer, size_t& offset) {
    AudioSourceComponent component;
    component.sound_name = deserialize<std::string>(buffer, offset);
    component.play_on_start = deserialize<bool>(buffer, offset);
    component.loop = deserialize<bool>(buffer, offset);
    component.destroy_entity_on_finish = deserialize<bool>(buffer, offset);
    return component;
}

/** Pattern Component */
inline void serialize(std::vector<uint8_t>& buffer, const PatternComponent& component) {
    serialize(buffer, component.type);
    serialize(buffer, static_cast<uint32_t>(component.waypoints.size()));
    for (const auto& pair : component.waypoints) {
        serialize(buffer, pair.first);
        serialize(buffer, pair.second);
    }
    serialize(buffer, component.current_index);
    serialize(buffer, component.speed);
    serialize(buffer, component.loop);
    serialize(buffer, component.is_active);
    serialize(buffer, component.amplitude);
    serialize(buffer, component.frequency);
    serialize(buffer, component.time_elapsed);
}

inline PatternComponent deserialize_pattern_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    PatternComponent component;
    component.type = deserialize<PatternComponent::PatternType>(buffer, offset);
    uint32_t waypoints_size = deserialize<uint32_t>(buffer, offset);

    // Safety: corrupted packets can contain absurd sizes and crash on resize.
    constexpr uint32_t MAX_WAYPOINTS = 512;
    if (waypoints_size > MAX_WAYPOINTS) {
        waypoints_size = 0;
    }

    component.waypoints.resize(waypoints_size);
    for (uint32_t i = 0; i < waypoints_size; ++i) {
        component.waypoints[i].first = deserialize<float>(buffer, offset);
        component.waypoints[i].second = deserialize<float>(buffer, offset);
    }
    component.current_index = deserialize<int>(buffer, offset);
    component.speed = deserialize<float>(buffer, offset);
    component.loop = deserialize<bool>(buffer, offset);
    component.is_active = deserialize<bool>(buffer, offset);
    component.amplitude = deserialize<float>(buffer, offset);
    component.frequency = deserialize<float>(buffer, offset);
    component.time_elapsed = deserialize<float>(buffer, offset);
    return component;
}

/** Health Component */
inline void serialize(std::vector<uint8_t>& buffer, const HealthComponent& component) {
    serialize(buffer, component.max_hp);
    serialize(buffer, component.current_hp);
}

inline HealthComponent deserialize_health_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    HealthComponent component;
    component.max_hp = deserialize<int>(buffer, offset);
    component.current_hp = deserialize<int>(buffer, offset);
    return component;
}

/** Resource Component */
inline void serialize(std::vector<uint8_t>& buffer, const ResourceStat& stat) {
    serialize(buffer, stat.current);
    serialize(buffer, stat.max);
    serialize(buffer, stat.regenRate);
}

inline ResourceStat deserialize_resource_stat(const std::vector<uint8_t>& buffer, size_t& offset) {
    ResourceStat stat;
    stat.current = deserialize<float>(buffer, offset);
    stat.max = deserialize<float>(buffer, offset);
    stat.regenRate = deserialize<float>(buffer, offset);
    return stat;
}

inline void serialize(std::vector<uint8_t>& buffer, const ResourceComponent& component) {
    serialize(buffer, component.resources);
}

inline ResourceComponent deserialize_resource_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    ResourceComponent component;
    component.resources = deserialize_map<std::string, ResourceStat>(buffer, offset);
    return component;
}

/** Transform Component */
inline void serialize(std::vector<uint8_t>& buffer, const transform_component_s& component) {
    serialize(buffer, component.x);
    serialize(buffer, component.y);
    serialize(buffer, component.scale_x);
    serialize(buffer, component.scale_y);
    serialize(buffer, component.rotation);
}

inline transform_component_s deserialize_transform_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    transform_component_s component;
    component.x = deserialize<float>(buffer, offset);
    component.y = deserialize<float>(buffer, offset);
    component.scale_x = deserialize<float>(buffer, offset);
    component.scale_y = deserialize<float>(buffer, offset);
    component.rotation = deserialize<float>(buffer, offset);
    return component;
}

/** Velocity2D Component */
inline void serialize(std::vector<uint8_t>& buffer, const Velocity2D& component) {
    serialize(buffer, component.vx);
    serialize(buffer, component.vy);
}

inline Velocity2D deserialize_velocity_2d(const std::vector<uint8_t>& buffer, size_t& offset) {
    Velocity2D component;
    component.vx = deserialize<float>(buffer, offset);
    component.vy = deserialize<float>(buffer, offset);
    return component;
}

/** BoxCOllision Component */
inline void serialize(std::vector<uint8_t>& buffer, const BoxCollisionComponent& component) {
    serialize(buffer, component.tagCollision);
}

inline BoxCollisionComponent deserialize_box_collision_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    BoxCollisionComponent component;
    component.tagCollision = deserialize_vector<std::string>(buffer, offset);
    return component;
}

/** Sprite2D Component */
inline void serialize(std::vector<uint8_t>& buffer, const rect& r) {
    serialize(buffer, r.x);
    serialize(buffer, r.y);
    serialize(buffer, r.width);
    serialize(buffer, r.height);
}

inline rect deserialize_rect(const std::vector<uint8_t>& buffer, size_t& offset) {
    rect r;
    r.x = deserialize<float>(buffer, offset);
    r.y = deserialize<float>(buffer, offset);
    r.width = deserialize<float>(buffer, offset);
    r.height = deserialize<float>(buffer, offset);
    return r;
}

inline void serialize(std::vector<uint8_t>& buffer, const sprite2D_component_s& component,
                      ResourceManager<TextureAsset>& resourceManager) {
    auto name = resourceManager.get_name(component.handle);
    if (name) {
        serialize(buffer, name.value());
    } else {
        serialize(buffer, std::string(""));
    }
    serialize(buffer, component.dimension);
    serialize(buffer, component.is_animated);
    serialize(buffer, component.frames);
    serialize(buffer, component.reverse_animation);
    serialize(buffer, component.loop_animation);
    serialize(buffer, component.animation_speed);
    serialize(buffer, component.current_animation_frame);
    serialize(buffer, component.last_animation_update);
    serialize(buffer, component.z_index);
    serialize(buffer, component.lastUpdateTime);
}

inline sprite2D_component_s deserialize_sprite_2d_component(const std::vector<uint8_t>& buffer, size_t& offset,
                                                            ResourceManager<TextureAsset>& resourceManager) {
    sprite2D_component_s component;
    std::string name = deserialize<std::string>(buffer, offset);
    if (!name.empty()) {
        if (resourceManager.is_loaded(name)) {
            component.handle = resourceManager.get_handle(name).value();
        } else {
            // Load the texture if not already loaded (critical for client-side rendering)
#if defined(CLIENT_BUILD)
            TextureAsset texture;
            std::string path = name;
            bool loaded = texture.loadFromFile(path);
            if (!loaded) {
                path = "../" + name;
                loaded = texture.loadFromFile(path);
            }

            if (!loaded) {
                std::cerr << "Client: CRITICAL ERROR: Failed to load texture: " << name << " from any path."
                          << std::endl;
            }
            component.handle = resourceManager.load(name, texture);
#else
            component.handle = resourceManager.load(name, TextureAsset(name));
#endif
        }
    }
    component.dimension = deserialize_rect(buffer, offset);
    component.is_animated = deserialize<bool>(buffer, offset);
    uint32_t frames_size = deserialize<uint32_t>(buffer, offset);

    // Safety: avoid std::length_error on corrupted packets
    constexpr uint32_t MAX_FRAMES = 256;
    if (frames_size > MAX_FRAMES) {
        frames_size = 0;
    }

    component.frames.resize(frames_size);
    for (uint32_t i = 0; i < frames_size; ++i) {
        component.frames[i] = deserialize_rect(buffer, offset);
    }
    component.reverse_animation = deserialize<bool>(buffer, offset);
    component.loop_animation = deserialize<bool>(buffer, offset);
    component.animation_speed = deserialize<float>(buffer, offset);
    component.current_animation_frame = deserialize<int>(buffer, offset);
    component.last_animation_update = deserialize<float>(buffer, offset);
    component.z_index = deserialize<int>(buffer, offset);
    component.lastUpdateTime = deserialize<float>(buffer, offset);
    return component;
}

/** Tag Component */
inline void serialize(std::vector<uint8_t>& buffer, const TagComponent& component) {
    serialize(buffer, component.tags);
}

inline TagComponent deserialize_tag_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    TagComponent component;
    component.tags = deserialize_vector<std::string>(buffer, offset);
    return component;
}

/** Text Component */
inline void serialize(std::vector<uint8_t>& buffer, const sf::Color& color) {
    serialize(buffer, color.r);
    serialize(buffer, color.g);
    serialize(buffer, color.b);
    serialize(buffer, color.a);
}

inline sf::Color deserialize_color(const std::vector<uint8_t>& buffer, size_t& offset) {
    sf::Color color;
    color.r = deserialize<uint8_t>(buffer, offset);
    color.g = deserialize<uint8_t>(buffer, offset);
    color.b = deserialize<uint8_t>(buffer, offset);
    color.a = deserialize<uint8_t>(buffer, offset);
    return color;
}

inline void serialize(std::vector<uint8_t>& buffer, const TextComponent& component) {
    serialize(buffer, component.text);
    serialize(buffer, component.fontPath);
    serialize(buffer, component.characterSize);
    serialize(buffer, component.color);
    serialize(buffer, component.x);
    serialize(buffer, component.y);
}

inline TextComponent deserialize_text_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    TextComponent component;
    component.text = deserialize<std::string>(buffer, offset);
    component.fontPath = deserialize<std::string>(buffer, offset);
    component.characterSize = deserialize<unsigned int>(buffer, offset);
    component.color = deserialize_color(buffer, offset);
    component.x = deserialize<float>(buffer, offset);
    component.y = deserialize<float>(buffer, offset);
    return component;
}

/** Projectile Component */
/** Projectile Component */
inline void serialize(std::vector<uint8_t>& buffer, const ProjectileComponent& component) {
    serialize(buffer, component.owner_id);
}

inline ProjectileComponent deserialize_projectile_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    ProjectileComponent component;
    component.owner_id = deserialize<int>(buffer, offset);
    return component;
}

/** Team Component */
inline void serialize(std::vector<uint8_t>& buffer, const TeamComponent& component) {
    serialize(buffer, component.team);
}

inline TeamComponent deserialize_team_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    TeamComponent component;
    component.team = deserialize<TeamComponent::Team>(buffer, offset);
    return component;
}

/** DamageOnCollision Component */
inline void serialize(std::vector<uint8_t>& buffer, const DamageOnCollision& component) {
    serialize(buffer, component.damage_value);
}

inline DamageOnCollision deserialize_damage_on_collision(const std::vector<uint8_t>& buffer, size_t& offset) {
    DamageOnCollision component;
    component.damage_value = deserialize<int>(buffer, offset);
    return component;
}

/** NetworkIdentity Component */
inline void serialize(std::vector<uint8_t>& buffer, const NetworkIdentity& component) {
    serialize(buffer, component.guid);
    serialize(buffer, component.ownerId);
}

inline NetworkIdentity deserialize_network_identity(const std::vector<uint8_t>& buffer, size_t& offset) {
    NetworkIdentity component;
    component.guid = deserialize<uint32_t>(buffer, offset);
    component.ownerId = deserialize<uint32_t>(buffer, offset);
    return component;
}

/** Scroll Component */
inline void serialize(std::vector<uint8_t>& buffer, const Scroll& component) {
    serialize(buffer, component.scroll_speed_x);
    serialize(buffer, component.scroll_speed_y);
    serialize(buffer, component.is_paused);
}

inline Scroll deserialize_scroll_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    Scroll component;
    component.scroll_speed_x = deserialize<float>(buffer, offset);
    component.scroll_speed_y = deserialize<float>(buffer, offset);
    component.is_paused = deserialize<bool>(buffer, offset);
    return component;
}

/** Bakcgroud Component */
inline void serialize(std::vector<uint8_t>& buffer, const BackgroundComponent& component,
                      ResourceManager<TextureAsset>& resourceManager) {
    auto name = resourceManager.get_name(component.texture_handle);
    if (name) {
        serialize(buffer, name.value());
    } else {
        serialize(buffer, std::string(""));
    }
    serialize(buffer, component.x_offset);
    serialize(buffer, component.scroll_speed);
}

inline BackgroundComponent deserialize_background_component(const std::vector<uint8_t>& buffer, size_t& offset,
                                                            ResourceManager<TextureAsset>& resourceManager) {
    BackgroundComponent component;
    std::string name = deserialize<std::string>(buffer, offset);
    if (!name.empty()) {
        if (resourceManager.is_loaded(name)) {
            component.texture_handle = resourceManager.get_handle(name).value();
        } else {
            // Load the texture if not already loaded (critical for client-side rendering)
            component.texture_handle = resourceManager.load(name, TextureAsset(name));
        }
    }
    component.x_offset = deserialize<float>(buffer, offset);
    component.scroll_speed = deserialize<float>(buffer, offset);
    return component;
}

/** Game Timer Component */

inline void serialize(std::vector<uint8_t>& buffer, const ::GameTimerComponent& component) {
    serialize(buffer, component.elapsed_time);
}

inline ::GameTimerComponent deserialize_game_timer_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    ::GameTimerComponent component;
    component.elapsed_time = deserialize<float>(buffer, offset);
    return component;
}

/** ShooterComponent */
inline void serialize(std::vector<uint8_t>& buffer, const ShooterComponent& component) {
    serialize(buffer, static_cast<int>(component.type));
    serialize(buffer, static_cast<int>(component.pattern));
    serialize(buffer, component.is_shooting);
    serialize(buffer, component.trigger_pressed);
    serialize(buffer, component.fire_rate);
    serialize(buffer, component.last_shot);
    serialize(buffer, component.projectile_damage);
    serialize(buffer, component.use_pod_laser);
}

inline ShooterComponent deserialize_shooter_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    ShooterComponent component;
    component.type = static_cast<ShooterComponent::ProjectileType>(deserialize<int>(buffer, offset));
    component.pattern = static_cast<ShooterComponent::ShootPattern>(deserialize<int>(buffer, offset));
    component.is_shooting = deserialize<bool>(buffer, offset);
    component.trigger_pressed = deserialize<bool>(buffer, offset);
    component.fire_rate = deserialize<double>(buffer, offset);
    component.last_shot = deserialize<double>(buffer, offset);
    component.projectile_damage = deserialize<int>(buffer, offset);
    component.use_pod_laser = deserialize<bool>(buffer, offset);
    return component;
}

/** ChargedShotComponent */
inline void serialize(std::vector<uint8_t>& buffer, const ChargedShotComponent& component) {
    serialize(buffer, component.is_charging);
    serialize(buffer, component.charge_time);
    serialize(buffer, component.min_charge_time);
    serialize(buffer, component.max_charge_time);
    serialize(buffer, component.medium_charge);
}

inline ChargedShotComponent deserialize_charged_shot_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    ChargedShotComponent component;
    component.is_charging = deserialize<bool>(buffer, offset);
    component.charge_time = deserialize<float>(buffer, offset);
    component.min_charge_time = deserialize<float>(buffer, offset);
    component.max_charge_time = deserialize<float>(buffer, offset);
    component.medium_charge = deserialize<float>(buffer, offset);
    return component;
}

/** EnemySpawnComponent */
inline void serialize(std::vector<uint8_t>& buffer, const EnemySpawnComponent& component) {
    serialize(buffer, component.spawn_timer);
    serialize(buffer, component.spawn_interval);
    serialize(buffer, component.total_time);
    serialize(buffer, component.boss_spawned);
    serialize(buffer, component.boss_arrived);
    serialize(buffer, component.boss_intro_timer);
    serialize(buffer, component.wave_count);
    serialize(buffer, component.is_active);
    serialize(buffer, component.use_scripted_spawns);
    serialize(buffer, component.enemies_config_path);
    serialize(buffer, component.boss_config_path);
    serialize(buffer, component.game_config_path);
    serialize(buffer, component.random_seed);
    serialize(buffer, component.random_state);
}

inline EnemySpawnComponent deserialize_enemy_spawn_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    EnemySpawnComponent component;
    component.spawn_timer = deserialize<float>(buffer, offset);
    component.spawn_interval = deserialize<float>(buffer, offset);
    component.total_time = deserialize<float>(buffer, offset);
    component.boss_spawned = deserialize<bool>(buffer, offset);
    component.boss_arrived = deserialize<bool>(buffer, offset);
    component.boss_intro_timer = deserialize<float>(buffer, offset);
    component.wave_count = deserialize<int>(buffer, offset);
    component.is_active = deserialize<bool>(buffer, offset);
    component.use_scripted_spawns = deserialize<bool>(buffer, offset);
    component.enemies_config_path = deserialize<std::string>(buffer, offset);
    component.boss_config_path = deserialize<std::string>(buffer, offset);
    component.game_config_path = deserialize<std::string>(buffer, offset);
    component.random_seed = deserialize<unsigned int>(buffer, offset);
    component.random_state = deserialize<int>(buffer, offset);
    return component;
}

/** PodComponent */
inline void serialize(std::vector<uint8_t>& buffer, const PodComponent& component) {
    serialize(buffer, static_cast<int>(component.state));
    serialize(buffer, static_cast<int>(component.owner_id));
    serialize(buffer, component.auto_fire_rate);
    serialize(buffer, component.last_shot_time);
    serialize(buffer, component.projectile_damage);
    serialize(buffer, component.float_time);
    serialize(buffer, component.wave_amplitude);
    serialize(buffer, component.wave_frequency);
    serialize(buffer, component.base_y);
}

inline PodComponent deserialize_pod_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    PodComponent component;
    component.state = static_cast<PodState>(deserialize<int>(buffer, offset));
    component.owner_id = deserialize<int>(buffer, offset);
    component.auto_fire_rate = deserialize<float>(buffer, offset);
    component.last_shot_time = deserialize<float>(buffer, offset);
    component.projectile_damage = deserialize<int>(buffer, offset);
    component.float_time = deserialize<float>(buffer, offset);
    component.wave_amplitude = deserialize<float>(buffer, offset);
    component.wave_frequency = deserialize<float>(buffer, offset);
    component.base_y = deserialize<float>(buffer, offset);
    return component;
}

/** PlayerPodComponent */
inline void serialize(std::vector<uint8_t>& buffer, const PlayerPodComponent& component) {
    serialize(buffer, component.has_pod);
    serialize(buffer, static_cast<int>(component.pod_entity));
    serialize(buffer, component.pod_attached);
    serialize(buffer, component.detach_requested);
    serialize(buffer, component.last_known_hp);
    serialize(buffer, component.pod_laser_fire_rate);
    serialize(buffer, component.pod_laser_cooldown);
}

inline PlayerPodComponent deserialize_player_pod_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    PlayerPodComponent component;
    component.has_pod = deserialize<bool>(buffer, offset);
    component.pod_entity = deserialize<int>(buffer, offset);
    component.pod_attached = deserialize<bool>(buffer, offset);
    component.detach_requested = deserialize<bool>(buffer, offset);
    component.last_known_hp = deserialize<int>(buffer, offset);
    component.pod_laser_fire_rate = deserialize<float>(buffer, offset);
    component.pod_laser_cooldown = deserialize<float>(buffer, offset);
    return component;
}

/** BehaviorComponent */
inline void serialize(std::vector<uint8_t>& buffer, const BehaviorComponent& component) {
    serialize(buffer, component.shoot_at_player);
    serialize(buffer, component.follow_player);
    serialize(buffer, component.follow_speed);
}

inline BehaviorComponent deserialize_behavior_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    BehaviorComponent component;
    component.shoot_at_player = deserialize<bool>(buffer, offset);
    component.follow_player = deserialize<bool>(buffer, offset);
    component.follow_speed = deserialize<float>(buffer, offset);
    return component;
}

/** BossComponent */
inline void serialize(std::vector<uint8_t>& buffer, const BossComponent& component) {
    serialize(buffer, component.has_arrived);
    serialize(buffer, component.target_x);
    serialize(buffer, static_cast<int>(component.current_state));
    serialize(buffer, component.state_timer);
    serialize(buffer, component.current_phase);
    serialize(buffer, component.is_enraged);
}

inline BossComponent deserialize_boss_component(const std::vector<uint8_t>& buffer, size_t& offset) {
    BossComponent component;
    component.has_arrived = deserialize<bool>(buffer, offset);
    component.target_x = deserialize<float>(buffer, offset);
    component.current_state = static_cast<BossComponent::BossState>(deserialize<int>(buffer, offset));
    component.state_timer = deserialize<float>(buffer, offset);
    component.current_phase = deserialize<int>(buffer, offset);
    component.is_enraged = deserialize<bool>(buffer, offset);
    return component;
}

/** BossSubEntityComponent */
inline void serialize(std::vector<uint8_t>& buffer, const BossSubEntityComponent& component) {
    serialize(buffer, component.boss_entity_id);
    serialize(buffer, static_cast<int>(component.type));
    serialize(buffer, component.sub_entity_index);
    serialize(buffer, component.is_active);
    serialize(buffer, component.is_destroyed);
    serialize(buffer, component.offset_x);
    serialize(buffer, component.offset_y);
    serialize(buffer, component.fire_timer);
    serialize(buffer, component.fire_rate);
}

inline BossSubEntityComponent deserialize_boss_sub_entity(const std::vector<uint8_t>& buffer, size_t& offset) {
    BossSubEntityComponent component;
    component.boss_entity_id = deserialize<int>(buffer, offset);
    component.type = static_cast<BossSubEntityComponent::SubEntityType>(deserialize<int>(buffer, offset));
    component.sub_entity_index = deserialize<int>(buffer, offset);
    component.is_active = deserialize<bool>(buffer, offset);
    component.is_destroyed = deserialize<bool>(buffer, offset);
    component.offset_x = deserialize<float>(buffer, offset);
    component.offset_y = deserialize<float>(buffer, offset);
    component.fire_timer = deserialize<float>(buffer, offset);
    component.fire_rate = deserialize<float>(buffer, offset);
    return component;
}

/** Rect2D */
inline void serialize(std::vector<uint8_t>& buffer, const Rect2D& r) {
    serialize(buffer, r.x);
    serialize(buffer, r.y);
    serialize(buffer, r.width);
    serialize(buffer, r.height);
}

inline Rect2D deserialize_rect2d(const std::vector<uint8_t>& buffer, size_t& offset) {
    Rect2D r;
    r.x = deserialize<int>(buffer, offset);
    r.y = deserialize<int>(buffer, offset);
    r.width = deserialize<int>(buffer, offset);
    r.height = deserialize<int>(buffer, offset);
    return r;
}

/** AnimationClip */
inline void serialize(std::vector<uint8_t>& buffer, const AnimationClip& clip,
                      ResourceManager<TextureAsset>& resourceManager) {
    auto name = resourceManager.get_name(clip.handle);
    if (name) {
        serialize(buffer, name.value());
    } else {
        serialize(buffer, std::string(""));
    }
    serialize(buffer, clip.frames);
    serialize(buffer, clip.frameDuration);
    serialize(buffer, static_cast<int>(clip.mode));
}

inline AnimationClip deserialize_animation_clip(const std::vector<uint8_t>& buffer, size_t& offset,
                                                ResourceManager<TextureAsset>& resourceManager) {
    AnimationClip clip;
    std::string name = deserialize<std::string>(buffer, offset);
    if (!name.empty()) {
        if (resourceManager.is_loaded(name)) {
            clip.handle = resourceManager.get_handle(name).value();
        } else {
#if defined(CLIENT_BUILD)
            TextureAsset texture;
            std::string path = name;
            bool loaded = texture.loadFromFile(path);
            if (!loaded) {
                path = "../" + name;
                loaded = texture.loadFromFile(path);
            }
            if (!loaded) {
                std::cerr << "Client: CRITICAL ERROR: Failed to load texture for animation: " << name << std::endl;
            }
            clip.handle = resourceManager.load(name, texture);
#else
            clip.handle = resourceManager.load(name, TextureAsset(name));
#endif
        }
    }

    // Manual vector deserialization for Rect2D to use our deserialize_rect2d
    uint32_t size = deserialize<uint32_t>(buffer, offset);
    if (size > 1024)
        size = 0;  // Safety cap
    clip.frames.resize(size);
    for (uint32_t i = 0; i < size; ++i) {
        clip.frames[i] = deserialize_rect2d(buffer, offset);
    }

    clip.frameDuration = deserialize<float>(buffer, offset);
    clip.mode = static_cast<AnimationMode>(deserialize<int>(buffer, offset));
    return clip;
}

/** AnimatedSprite2D */
inline void serialize(std::vector<uint8_t>& buffer, const AnimatedSprite2D& component,
                      ResourceManager<TextureAsset>& resourceManager) {
    serialize(buffer, static_cast<int>(component.layer));

    // Serialize animations map manually to handle the resource manager
    serialize(buffer, static_cast<uint32_t>(component.animations.size()));
    for (const auto& [name, clip] : component.animations) {
        serialize(buffer, name);
        serialize(buffer, clip, resourceManager);
    }

    serialize(buffer, component.currentAnimation);
    serialize(buffer, component.previousAnimation);
    serialize(buffer, static_cast<uint32_t>(component.currentFrameIndex));
    serialize(buffer, component.loopDirection);
    serialize(buffer, component.timer);
    serialize(buffer, component.playing);
    serialize(buffer, component.flipX);
    serialize(buffer, component.flipY);
}

inline AnimatedSprite2D deserialize_animated_sprite_2d(const std::vector<uint8_t>& buffer, size_t& offset,
                                                       ResourceManager<TextureAsset>& resourceManager) {
    AnimatedSprite2D component;
    component.layer = static_cast<RenderLayer>(deserialize<int>(buffer, offset));

    // Deserialize animations map
    uint32_t anims_size = deserialize<uint32_t>(buffer, offset);
    if (anims_size > 128)
        anims_size = 0;  // Safety

    for (uint32_t i = 0; i < anims_size; ++i) {
        std::string name = deserialize<std::string>(buffer, offset);
        AnimationClip clip = deserialize_animation_clip(buffer, offset, resourceManager);
        component.animations[name] = clip;
    }

    component.currentAnimation = deserialize<std::string>(buffer, offset);
    component.previousAnimation = deserialize<std::string>(buffer, offset);
    component.currentFrameIndex = deserialize<uint32_t>(buffer, offset);
    component.loopDirection = deserialize<int>(buffer, offset);
    component.timer = deserialize<float>(buffer, offset);
    component.playing = deserialize<bool>(buffer, offset);
    component.flipX = deserialize<bool>(buffer, offset);
    component.flipY = deserialize<bool>(buffer, offset);

    return component;
}

}  // namespace serialize
