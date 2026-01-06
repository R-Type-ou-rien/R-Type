#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"
#include "../../Components/team_component.hpp"
#include "../../Systems/ai_behavior.hpp"
#include "../../Systems/score.hpp"

void KamikazeSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {0.0f, 0.0f});

    registry.addComponent<HealthComponent>(id, {config.hp.value(), config.hp.value(), 0.0f, 0.3f});
    registry.addComponent<TeamComponent>(id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(id, {config.damage.value()});
    registry.addComponent<ScoreValueComponent>(id, {config.score_value.value()});

    AIBehaviorComponent behavior;
    behavior.shoot_at_player = false;
    behavior.follow_player = true;
    behavior.follow_speed = config.speed.value();
    registry.addComponent<AIBehaviorComponent>(id, behavior);

    handle_t<TextureAsset> handle = context.texture_manager.load(config.sprite_path.value(), TextureAsset(config.sprite_path.value()));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {static_cast<float>(config.sprite_x.value()), static_cast<float>(config.sprite_y.value()), 
                             static_cast<float>(config.sprite_w.value()), static_cast<float>(config.sprite_h.value())};
    
    sprite_info.z_index = 1;
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("AI");
    registry.addComponent<TagComponent>(id, tags);
}
