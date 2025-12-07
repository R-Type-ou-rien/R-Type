#include "health_feature/health.hpp"
#include "transform_component/transform.hpp"

void HealthSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<HealthComponent>();

    for (auto entity : entities) {
        auto& health = registry.getComponent<HealthComponent>(entity);
        if (health.current_hp <= 0) {
            if (registry.hasComponent<LifeComponent>(entity)) {
                auto& life = registry.getComponent<LifeComponent>(entity);
                if (life.lives_remaining > 0) {
                    life.lives_remaining--;
                    health.current_hp = health.max_hp;
                    std::cout << "Entity " << entity << " lost a life. Lives remaining: " << life.lives_remaining
                              << std::endl;
                    if (registry.hasComponent<TransformComponent>(entity)) {
                        auto& transform = registry.getComponent<TransformComponent>(entity);
                        transform.x = 100;
                        transform.y = 300;
                    }
                    continue;
                }
            }
            std::cout << "Entity " << entity << " has died." << std::endl;
        }
    }
}
