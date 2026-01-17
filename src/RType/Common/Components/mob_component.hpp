/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Mob Component - Flexible and configurable mob entity components and factories
*/

#pragma once

#include <string>
#include <vector>
#include <utility>
#include "config.hpp"
#include "team_component.hpp"
#include "../Systems/damage.hpp"
#include "../Systems/health.hpp"
#include "../Systems/shooter.hpp"
#include "../Systems/behavior.hpp"
#include "../Systems/score.hpp"
#include "Components/StandardComponents.hpp"

namespace MobDefaults {
namespace Health {
constexpr float INVINCIBILITY_TIME = 0.0f;
constexpr float DAMAGE_FLASH_DURATION = 0.5f;
constexpr float DAMAGE_FLASH_DURATION_TANK = 0.8f;
constexpr float DAMAGE_FLASH_DURATION_KAMIKAZE = 0.3f;
}  // namespace Health

namespace Shooter {
constexpr float FIRE_RATE = 1.0f;
constexpr int PROJECTILE_DAMAGE = 15;
constexpr float PROJECTILE_SCALE = 3.0f;
constexpr float LAST_SHOT = 0.0f;
}  // namespace Shooter

namespace Behavior {
constexpr float FOLLOW_SPEED = 100.0f;
constexpr float FOLLOW_SPEED_KAMIKAZE = 150.0f;
constexpr float FOLLOW_SPEED_TANK = 50.0f;
}  // namespace Behavior

namespace Sprite {
constexpr int Z_INDEX = 1;
constexpr float ANIMATION_SPEED = 0.1f;
constexpr int ANIMATION_FRAMES = 1;
}  // namespace Sprite

namespace Obstacle {
constexpr float VELOCITY_X = -120.0f;
constexpr int HP = 1000;
constexpr int DAMAGE = 100;
constexpr float SCALE = 2.0f;
constexpr float SPRITE_X = 261.0f;
constexpr float SPRITE_Y = 165.0f;
constexpr float SPRITE_W = 32.0f;
constexpr float SPRITE_H = 16.0f;
inline const char* SPRITE_PATH = "src/RType/Common/content/sprites/wall-level1.gif";
}  // namespace Obstacle

namespace CollisionTags {
inline const char* FRIENDLY_PROJECTILE = "FRIENDLY_PROJECTILE";
inline const char* ENEMY_PROJECTILE = "ENEMY_PROJECTILE";
inline const char* PLAYER = "PLAYER";
}  // namespace CollisionTags

namespace EntityTags {
inline const char* AI = "AI";
inline const char* OBSTACLE = "OBSTACLE";
inline const char* WALL = "WALL";
}  // namespace EntityTags
}  // namespace MobDefaults

class MobComponentFactory {
   public:
    static HealthComponent createHealth(const EntityConfig& config,
                                        float damage_flash_duration = MobDefaults::Health::DAMAGE_FLASH_DURATION) {
        return HealthComponent{config.hp.value(), config.hp.value(), MobDefaults::Health::INVINCIBILITY_TIME,
                               damage_flash_duration};
    }

    static ShooterComponent createShooter(const EntityConfig& config) {
        ShooterComponent shooter;
        shooter.type = ShooterComponent::NORMAL;
        shooter.is_shooting = true;
        shooter.fire_rate = config.fire_rate.value_or(MobDefaults::Shooter::FIRE_RATE);
        shooter.last_shot = MobDefaults::Shooter::LAST_SHOT;
        shooter.projectile_damage = config.projectile_damage.value_or(MobDefaults::Shooter::PROJECTILE_DAMAGE);
        shooter.projectile_scale = config.projectile_scale.value_or(MobDefaults::Shooter::PROJECTILE_SCALE);
        shooter.pattern = parseShootPattern(config.shoot_pattern);
        return shooter;
    }

    static BehaviorComponent createBehavior(const EntityConfig& config,
                                            float default_follow_speed = MobDefaults::Behavior::FOLLOW_SPEED) {
        BehaviorComponent behavior;
        behavior.shoot_at_player = config.shoot_at_player.value_or(false);
        behavior.follow_player = config.follow_player.value_or(false);
        behavior.follow_speed = config.speed.value_or(default_follow_speed);
        return behavior;
    }

    static sprite2D_component_s createSprite(const EntityConfig& config, handle_t<TextureAsset> handle,
                                             int z_index = MobDefaults::Sprite::Z_INDEX) {
        sprite2D_component_s sprite;
        sprite.handle = handle;
        sprite.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                            static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};
        sprite.z_index = z_index;
        return sprite;
    }

    static BoxCollisionComponent createEnemyCollision(const EntityConfig& config) {
        BoxCollisionComponent collision;
        if (!config.collision_tags.empty()) {
            for (const auto& tag : config.collision_tags) {
                collision.tagCollision.push_back(tag);
            }
        } else {
            collision.tagCollision.push_back(MobDefaults::CollisionTags::FRIENDLY_PROJECTILE);
            collision.tagCollision.push_back(MobDefaults::CollisionTags::ENEMY_PROJECTILE);
            collision.tagCollision.push_back(MobDefaults::CollisionTags::PLAYER);
        }
        return collision;
    }

    static TagComponent createAITags(const std::vector<std::string>& additional_tags = {}) {
        TagComponent tags;
        tags.tags.push_back(MobDefaults::EntityTags::AI);
        for (const auto& tag : additional_tags) {
            tags.tags.push_back(tag);
        }
        return tags;
    }

    static TagComponent createObstacleTags() {
        TagComponent tags;
        tags.tags.push_back(MobDefaults::EntityTags::OBSTACLE);
        tags.tags.push_back(MobDefaults::EntityTags::WALL);
        return tags;
    }

    static PatternComponent createPattern(const EntityConfig& config) {
        PatternComponent pat;
        pat.type = parsePatternType(config.pattern);
        pat.speed = config.speed.value();
        pat.amplitude = config.amplitude.value_or(50.0f);
        pat.frequency = config.frequency.value_or(2.0f);
        pat.is_active = true;
        return pat;
    }

    static void applyScale(Registry& registry, Entity entity, const EntityConfig& config) {
        if (config.scale.has_value()) {
            auto& transform = registry.getComponent<transform_component_s>(entity);
            transform.scale_x = config.scale.value();
            transform.scale_y = config.scale.value();
        }
    }

    static std::pair<int, float> getAnimationParams(const EntityConfig& config) {
        return {config.animation_frames.value_or(MobDefaults::Sprite::ANIMATION_FRAMES),
                config.animation_speed.value_or(MobDefaults::Sprite::ANIMATION_SPEED)};
    }

   private:
    static ShooterComponent::ShootPattern parseShootPattern(const std::optional<std::string>& pattern_str) {
        if (!pattern_str.has_value()) {
            return ShooterComponent::STRAIGHT;
        }
        const std::string& pattern = pattern_str.value();
        if (pattern == "AIM_PLAYER") {
            return ShooterComponent::AIM_PLAYER;
        } else if (pattern == "SPREAD") {
            return ShooterComponent::SPREAD;
        }
        return ShooterComponent::STRAIGHT;
    }

    static PatternComponent::PatternType parsePatternType(const std::optional<std::string>& pattern_str) {
        if (!pattern_str.has_value()) {
            return PatternComponent::LINEAR;
        }
        const std::string& pattern = pattern_str.value();
        if (pattern == "SINUSOIDAL") {
            return PatternComponent::SINUSOIDAL;
        } else if (pattern == "ZIGZAG") {
            return PatternComponent::ZIGZAG;
        } else if (pattern == "CIRCULAR") {
            return PatternComponent::CIRCULAR;
        }
        return PatternComponent::LINEAR;
    }
};
