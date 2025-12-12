#include "spawn.hpp"

#include <iostream>
#include <vector>

#include "ecs/common/Components/Components.hpp"
#include "ecs/common/box_collision/box_collision.hpp"
#include "ecs/common/damage_feature/damage.hpp"
#include "ecs/common/health_feature/health.hpp"
#include "ecs/common/shoot_feature/shooter.hpp"
#include "ecs/common/team_component/team_component.hpp"

SpawnSystem::SpawnSystem() {
    _factories[TypeEntityComponent::PLAYER] = [](Registry& r, Entity e) {
        r.addComponent(e, TeamComponent{TeamComponent::ALLY});
        r.addComponent(e, HealthComponent{100, 100});
        r.addComponent(e, BoxCollisionComponent{});
        r.addComponent(e, InputControl{});
    };

    _factories[TypeEntityComponent::ENEMY_BASIC] = [](Registry& r, Entity e) {
        r.addComponent(e, TeamComponent{TeamComponent::ENEMY});
        r.addComponent(e, HealthComponent{20, 20});
        r.addComponent(e, DamageOnCollision{10});
        r.addComponent(e, BoxCollisionComponent{});
        r.addComponent(e, sprite2D_component_s{{}, sf::IntRect({0, 0}, {33, 36})});
    };

    _factories[TypeEntityComponent::ENEMY_SHOOTER] = [](Registry& r, Entity e) {
        r.addComponent(e, TeamComponent{TeamComponent::ENEMY});
        r.addComponent(e, HealthComponent{50, 50});
        r.addComponent(e, DamageOnCollision{20});
        r.addComponent(e, BoxCollisionComponent{});
        r.addComponent(e, sprite2D_component_s{{}, sf::IntRect({50, 0}, {50, 50})});
        r.addComponent(e, ShooterComponent{});
    };
}

void SpawnSystem::update(Registry& registry, system_context context) {
    auto& spawners = registry.getEntities<SpawnComponent>();
    std::vector<Entity> to_destroy;

    for (auto entity : spawners) {
        auto& spawn = registry.getComponent<SpawnComponent>(entity);
        spawn.time_until_spawn -= context.dt;
        if (spawn.time_until_spawn <= 0) {
            create_entity(registry, spawn);
            to_destroy.push_back(entity);
        }
    }
    for (auto e : to_destroy) {
        registry.destroyEntity(e);
    }
}

void SpawnSystem::create_entity(Registry& registry, const SpawnComponent& info) {
    Entity e = registry.createEntity();

    registry.addComponent(e, info.transform);
    registry.addComponent(e, info.initial_velocity);

    auto it = _factories.find(info.type.type);

    if (it != _factories.end()) {
        it->second(registry, e);
    } else {
        std::cerr << "SpawnSystem: Unknown entity type: " << info.type.type << std::endl;
    }
}
