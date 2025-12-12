#pragma once

#include <vector>

#include "ecs/common/ISystem.hpp"
#include "ecs/common/Registry/registry.hpp"

struct HierarchyComponent {
    int parent_id;
    std::vector<int> children_id;
    bool is_dead;
};

class HierarchySystem : public ISystem {
   public:
    HierarchySystem() = default;
    ~HierarchySystem() = default;

    void update(Registry& registry, system_context context) override;
};
