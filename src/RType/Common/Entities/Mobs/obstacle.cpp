#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"

void ObstacleSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-120.0f, 0.0f});

    registry.addComponent<HealthComponent>(id, {150, 150, 0.0f, 0.0f});
    registry.addComponent<DamageOnCollision>(id, {30});

    handle_t<TextureAsset> handle = context.texture_manager.load("content/sprites/r-typesheet14.gif",
                                                                 TextureAsset("content/sprites/r-typesheet14.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {0, 0, 32, 32};
    
    sprite_info.z_index = 1;
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("PLAYER");
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("ENEMY_PROJECTILE");
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("OBSTACLE");
    registry.addComponent<TagComponent>(id, tags);
}
