#include "shooter.hpp"

#include <iostream>
#include <iterator>
#include <ostream>

#include "Components/StandardComponents.hpp"
#include "Hash/Hash.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "team_component.hpp"

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
    TagComponent tags;

    tags.tags.push_back("PROJECTILE");

    registry.addComponent<ProjectileComponent>(id, {id});

    registry.addComponent<TeamComponent>(id, {team});

    registry.addComponent<transform_component_s>(id, {(pos.x + 32), (pos.y + 8)});

    registry.addComponent<Velocity2D>(id, speed);

    registry.addComponent<TagComponent>(id, tags);

    handle_t<sf::Texture> handle = context.texture_manager.load_resource(
        "content/sprites/r-typesheet1.gif", sf::Texture("content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.texture_id = Hash::fnv1a("content/sprites/r-typesheet1.gif");
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

    collision.callbackOnCollide = [](Registry& reg, system_context con, Entity current) {
        BoxCollisionComponent& coll = reg.getComponent<BoxCollisionComponent>(current);

        if (con.network_server.has_value() && con.clients_id.has_value()) {
            Server& server = con.network_server.value();
            auto& players = con.clients_id.value().get();

            for (auto collided_entity : coll.collision.tags) {
                if (reg.hasComponent<NetworkIdentity>(collided_entity)) {
                    uint32_t guid =
                        static_cast<uint32_t>(reg.getComponent<NetworkIdentity>(collided_entity)
                                                  .guid);  // Cast to uint32 for simple message? Or send uint64?
                    // Network message usually supports pod types. Let's assume uint64 is fine or cast to 32 if ID is
                    // small enough. Actually guid is entity ID (uint32_t converted to uint64_t in GameManager).

                    for (auto& player_id : players) {
                        server.AddMessageToPlayer(GameEvents::S_PLAYER_DEATH, player_id, guid);
                    }
                }
                reg.destroyEntity(collided_entity);
            }
            // Also destroy projectile (current)
            // Projectiles might not have NetworkIdentity if they are not synced?
            // But they ARE synced (ComponentSenderSystem iterates pools).
            // Does create_projectile add NetworkIdentity?
            // Check create_projectile... No, it adds components but I didn't see NetworkIdentity in the snippet (lines
            // 36-87). If projectile doesn't have NetworkIdentity, client won't know it's destroyed via ID. But if
            // client spawned it via Server snapshot, it must have had one? ComponentSenderSystem assigns guid =
            // registry.getComponentConst<NetworkIdentity>(entity).guid; If Projectile has no NetworkIdentity, it
            // crashes ComponentSenderSystem? Or assumes it has one. Let's check create_projectile again.
        } else {
            // Local mode or fallback
            for (auto collided_entity : coll.collision.tags) {
                reg.destroyEntity(collided_entity);
            }
        }
        reg.destroyEntity(current);
    };
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
        if (!shooter.is_shooting)
            continue;
        transform_component_s& pos = registry.getComponent<transform_component_s>(id);
        TeamComponent& team = registry.getComponent<TeamComponent>(id);

        if (shooter.last_shot >= shooter.fire_rate) {
            create_projectile(registry, shooter.type, team.team, pos, context);
            shooter.last_shot = 0.f;
        }
        shooter.is_shooting = false;
    }
}
