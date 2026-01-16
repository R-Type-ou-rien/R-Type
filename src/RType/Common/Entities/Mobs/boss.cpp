#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"
#include "../../Systems/shooter.hpp"
#include "../../Components/team_component.hpp"
#include "../../Systems/ai_behavior.hpp"
#include "../../Systems/score.hpp"

void BossSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    float windowWidth = 1920.0f;
    float windowHeight = 1080.0f;

    float boss_target_x = windowWidth - config.sprite_w.value() - 50.0f;

    registry.addComponent<transform_component_s>(
        id, {windowWidth + 50.0f, windowHeight / 2.0f - config.sprite_h.value() / 2.0f});
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
    registry.addComponent<ShooterComponent>(id, boss_shooter);

    handle_t<TextureAsset> handle =
        context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()),
                             static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};

    sprite_info.z_index = 2;
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("AI");
    tags.tags.push_back("BOSS");
    registry.addComponent<TagComponent>(id, tags);
}
