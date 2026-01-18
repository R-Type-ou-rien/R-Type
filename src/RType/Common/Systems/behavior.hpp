#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "PlayerBoundsSystem.hpp"
#include "../Components/behavior_component.hpp"

class BehaviorSystem : public ISystem {
   public:
    BehaviorSystem() = default;
    ~BehaviorSystem() = default;
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
