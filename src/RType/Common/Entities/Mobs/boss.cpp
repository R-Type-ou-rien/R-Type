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

void BossSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    float windowWidth = 1920.0f;
    float windowHeight = 1080.0f;

    float boss_width = config.sprite_w.value() * config.scale.value_or(1.0f);
    float boss_height = config.sprite_h.value() * config.scale.value_or(1.0f);
    float boss_target_x = windowWidth - boss_width - 100.0f;
    float start_x = windowWidth + 50.0f;
    float start_y = (windowHeight / 2.0f) - (boss_height / 2.0f);

    registry.addComponent<transform_component_s>(id, {start_x, start_y});
    registry.addComponent<Velocity2D>(id, {-config.speed.value(), 0.0f});

    registry.addComponent<HealthComponent>(id, {config.hp.value(), config.hp.value(), 0.0f, 2.0f});
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    BossComponent boss_comp;
    boss_comp.has_arrived = false;
    boss_comp.target_x = boss_target_x;
    registry.addComponent<BossComponent>(id, boss_comp);

    ShooterComponent boss_shooter;
    boss_shooter.type = ShooterComponent::NORMAL;
    boss_shooter.is_shooting = true;
    boss_shooter.fire_rate = config.fire_rate.value();
    boss_shooter.last_shot = 0.0f;
    boss_shooter.projectile_damage = config.projectile_damage.value_or(25);

    if (config.shoot_pattern.has_value()) {
        std::string pattern = config.shoot_pattern.value();
        if (pattern == "AIM_PLAYER") {
            boss_shooter.pattern = ShooterComponent::AIM_PLAYER;
        } else if (pattern == "SPREAD") {
            boss_shooter.pattern = ShooterComponent::SPREAD;
        } else {
            boss_shooter.pattern = ShooterComponent::STRAIGHT;
        }
    } else {
        boss_shooter.pattern = ShooterComponent::SPREAD;  // Boss tire en éventail par défaut
    }

    registry.addComponent<ShooterComponent>(id, boss_shooter);

    handle_t<TextureAsset> handle =
        context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                             static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};

    sprite_info.z_index = 2;
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
    tags.tags.push_back("BOSS");
    registry.addComponent<TagComponent>(id, tags);
}
