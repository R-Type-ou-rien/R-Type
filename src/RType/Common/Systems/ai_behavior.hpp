#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/ai_behavior_component.hpp"

class AIBehaviorSystem : public ISystem {
   public:
    AIBehaviorSystem() = default;
    ~AIBehaviorSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void updateFollowPlayer(Registry& registry, system_context context, Entity enemy, Entity player);
    void updateShootAtPlayer(Registry& registry, Entity enemy, Entity player);
};

class BoundsSystem : public ISystem {
   public:
    BoundsSystem() = default;
    ~BoundsSystem() = default;
    void update(Registry& registry, system_context context) override;
};
