#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

class LeaderboardSystem : public ISystem {
   public:
    LeaderboardSystem() = default;
    ~LeaderboardSystem() = default;
    void update(Registry& registry, system_context context) override;
    static Entity createLeaderboard(Registry& registry, bool victory);
};
