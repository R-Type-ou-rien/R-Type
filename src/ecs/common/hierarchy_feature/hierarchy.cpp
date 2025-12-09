#include "hierarchy.hpp"

#include <iostream>
#include <set>
#include <vector>

void HierarchySystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<HierarchyComponent>();
    std::vector<int> entities_dead;
    std::set<int> to_destroy;

    for (auto entity : entities) {
        if (!registry.hasComponent<HierarchyComponent>(entity))
            continue;
        auto& hierarchy = registry.getComponent<HierarchyComponent>(entity);
        if (hierarchy.is_dead) {
            entities_dead.push_back(entity);
            to_destroy.insert(entity);
        }
    }
    if (entities_dead.empty())
        return;
    for (size_t i = 0; i < entities_dead.size(); ++i) {
        int current_id = entities_dead[i];

        if (!registry.hasComponent<HierarchyComponent>(current_id))
            continue;
        auto& hierarchy = registry.getComponent<HierarchyComponent>(current_id);
        if (hierarchy.parent_id != -1) {
            if (to_destroy.find(hierarchy.parent_id) == to_destroy.end()) {
                to_destroy.insert(hierarchy.parent_id);
                entities_dead.push_back(hierarchy.parent_id);
                if (registry.hasComponent<HierarchyComponent>(hierarchy.parent_id)) {
                    registry.getComponent<HierarchyComponent>(hierarchy.parent_id).is_dead = true;
                }
            }
        }
        for (int child_id : hierarchy.children_id) {
            if (to_destroy.find(child_id) == to_destroy.end()) {
                to_destroy.insert(child_id);
                entities_dead.push_back(child_id);

                if (registry.hasComponent<HierarchyComponent>(child_id)) {
                    registry.getComponent<HierarchyComponent>(child_id).is_dead = true;
                }
            }
        }
    }
    for (int id : to_destroy) {
        registry.destroyEntity(id);
    }
}