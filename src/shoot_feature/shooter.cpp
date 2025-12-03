#include "shooter.hpp"
#include <iostream>
#include <ostream>

// namespace ECS {

VelocityComponent ShooterSystem::get_projectile_speed(ShooterComponent::Projectile type, TeamComponent::Team team) {
    VelocityComponent vel = {0, 0};
    double speed = 0.0;

    switch (type) {
        case ShooterComponent::NORMAL:
            speed = 10.0;
            break;
        case ShooterComponent::CHARG:
            speed = 5.0;
            break;
        case ShooterComponent::RED:
            speed = 12.0;
            break;
        case ShooterComponent::BLUE:
            speed = 15.0;
            break;
        default:
            speed = 10.0;
    }
    vel.vx = speed;
    vel.vy = 0;

    return vel;
}

void ShooterSystem::create_projectile(Registry& registry, ShooterComponent::Projectile type, TeamComponent::Team team,
                                      TransformComponent pos, VelocityComponent speed) {
    int id = registry.createEntity();
    ProjectileComponent projectile;
    TeamComponent team_component;
    TransformComponent transform;
    VelocityComponent velocity;

    projectile.owner_id = -1;
    registry.addComponent(id, projectile);

    team_component.team = team;
    registry.addComponent(id, team_component);

    transform.x = pos.x + 1.0;
    transform.y = pos.y;
    registry.addComponent(id, transform);

    velocity.vx = speed.vx;
    velocity.vy = speed.vy;
    registry.addComponent(id, velocity);

    std::cout << "Projectile create ID: " << id << "Type: " << type << std::endl;
}

void ShooterSystem::update(Registry& registry, float current_time) {
    auto& shootersIds = registry.getEntities<ShooterComponent>();
    std::cout << "ShooterSystem updated" << std::endl;
    for (auto id : shootersIds) {
        if (!registry.hasComponent<TransformComponent>(id)) {
            continue;
        }
        if (!registry.hasComponent<TeamComponent>(id)) {
            continue;
        }

        ShooterComponent& shooter = registry.getComponent<ShooterComponent>(id);
        TransformComponent& pos = registry.getComponent<TransformComponent>(id);
        TeamComponent& team = registry.getComponent<TeamComponent>(id);

        if (current_time - shooter.last_shot > shooter.fire_rate) {
            VelocityComponent projectile_velocity = get_projectile_speed(shooter.type, team.team);
            create_projectile(registry, shooter.type, team.team, pos, projectile_velocity);
        }
    }
}
// }  // namespace ECS
