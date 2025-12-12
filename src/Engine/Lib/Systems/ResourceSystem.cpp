#include "ResourceSystem.hpp"

#include "Components/StandardComponents.hpp"

void ResourceSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<ResourceComponent>();

    for (auto entity : entities) {
        ResourceComponent& resComp = registry.getComponent<ResourceComponent>(entity);
        for (auto& [type, stat] : resComp.resources) {
            if (stat.regenRate != 0.0f)
                stat.current += stat.regenRate * context.dt;

            if (stat.current > stat.max)
                stat.current = stat.max;

            if (stat.current <= 0.0f) {
                stat.current = 0.0f;
                if (resComp.empty_effects.find(type) != resComp.empty_effects.end())
                    resComp.empty_effects[type]();
            }
        }
    }
}