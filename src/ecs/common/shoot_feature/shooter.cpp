#include "shooter.hpp"

#include <iostream>
#include <ostream>

#include "ecs/common/Components/Components.hpp"

VelocityComponent ShooterSystem::get_projectile_speed(ShooterComponent::Projectile type, TeamComponent::Team team) {
    VelocityComponent vel = {0, 0};
    double speed = 0;

    switch (type) {
        case ShooterComponent::NORMAL:
            speed = 5;
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

void ShooterSystem::create_projectile(Registry& registry, ShooterComponent::Projectile type, TeamComponent::Team team,
                                      transform_component_s pos, VelocityComponent speed) {
    int id = registry.createEntity();
    ProjectileComponent projectile;
    TeamComponent team_component;
    transform_component_s transform;
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
        transform_component_s& pos = registry.getComponent<transform_component_s>(id);
        TeamComponent& team = registry.getComponent<TeamComponent>(id);

        if (context.dt - shooter.last_shot >= shooter.fire_rate) {
            VelocityComponent projectile_velocity = get_projectile_speed(shooter.type, team.team);
            create_projectile(registry, shooter.type, team.team, pos, projectile_velocity);
            shooter.last_shot = context.dt;
        }
    }
}
