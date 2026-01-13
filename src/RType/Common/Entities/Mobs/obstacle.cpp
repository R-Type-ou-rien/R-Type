#include "all_mobs.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "../../Systems/damage.hpp"
#include "../../Systems/health.hpp"
#include <string>

void ObstacleSpawner::spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) {
    Entity id = registry.createEntity();

    registry.addComponent<transform_component_s>(id, {x, y});
    registry.addComponent<Velocity2D>(id, {-120.0f, 0.0f}); 
    registry.addComponent<HealthComponent>(id, {150, 150, 0.0f, 0.0f});
    registry.addComponent<DamageOnCollision>(id, {30});
    std::string path = "src/RType/Common/content/sprites/wall-level1.gif";

    handle_t<TextureAsset> handle =
        context.texture_manager.load(path, TextureAsset(path));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;

    float rect_x = 261;
    float rect_y = 165;
    float rect_w = 32;
    float rect_h = 16;  

    sprite_info.dimension = {rect_x, rect_y, rect_w, rect_h};
    sprite_info.z_index = 1;
    
    registry.addComponent<sprite2D_component_s>(id, sprite_info);

    auto& transform = registry.getComponent<transform_component_s>(id);
    transform.scale_x = 2.0f;
    transform.scale_y = 2.0f;

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER"); 
    registry.addComponent<BoxCollisionComponent>(id, collision);

    TagComponent tags;
    tags.tags.push_back("OBSTACLE");
    tags.tags.push_back("WALL");
    registry.addComponent<TagComponent>(id, tags);
    registry.addComponent<NetworkIdentity>(id, {static_cast<uint32_t>(id), 0});
}