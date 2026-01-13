#include <string>
#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"
#include "../../Systems/animation_helper.hpp"
#include "../../Systems/shooter.hpp"
#include "../../Components/team_component.hpp"
#include "../../Systems/ai_behavior.hpp"
#include "../../Systems/score.hpp"

void TankSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-config.speed.value(), 0.0f});

    registry.addComponent<HealthComponent>(id, {config.hp.value(), config.hp.value(), 0.0f, 0.8f});
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    if (config.can_shoot.value_or(false)) {
        ShooterComponent shooter;
        shooter.type = ShooterComponent::NORMAL;
        shooter.is_shooting = true;
        shooter.fire_rate = config.fire_rate.value_or(1.0f);
        shooter.last_shot = 0.0f;
        shooter.projectile_damage = config.projectile_damage.value_or(15);
        shooter.projectile_scale = config.projectile_scale.value_or(3.0f);

        if (config.shoot_pattern.has_value()) {
            std::string pattern = config.shoot_pattern.value();
            if (pattern == "AIM_PLAYER") {
                shooter.pattern = ShooterComponent::AIM_PLAYER;
            } else if (pattern == "SPREAD") {
                shooter.pattern = ShooterComponent::SPREAD;
            } else {
                shooter.pattern = ShooterComponent::STRAIGHT;
            }
        } else {
            shooter.pattern = ShooterComponent::STRAIGHT;
        }
        registry.addComponent<ShooterComponent>(id, shooter);
    }

    if (config.shoot_at_player.value_or(false)) {
        AIBehaviorComponent behavior;
        behavior.shoot_at_player = true;
        behavior.follow_player = false;
        registry.addComponent<AIBehaviorComponent>(id, behavior);
    }

    handle_t<TextureAsset> handle =
        context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                             static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};

    sprite_info.z_index = 1;
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    int num_frames = config.animation_frames.value_or(1);
    float anim_speed = config.animation_speed.value_or(0.1f);
    if (num_frames > 1) {
        AnimationHelper::setupHorizontalAnimation(registry, id, config, num_frames, anim_speed);
    }

    if (config.scale.has_value()) {
        auto& transform = registry.getComponent<transform_component_s>(id);
        transform.scale_x = config.scale.value();
        transform.scale_y = config.scale.value();
    }

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("AI");
    registry.addComponent<TagComponent>(id, tags);

    // Add NetworkIdentity for network replication
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});
}
