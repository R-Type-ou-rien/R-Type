#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/pod_component.hpp"
#include "../Components/team_component.hpp"

class PodSystem : public ISystem {
   public:
    PodSystem() = default;
    ~PodSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void spawnPod(Registry& registry, system_context context);
    void handlePodCollection(Registry& registry);
    void updateAttachedPodPosition(Registry& registry);
    void updateFloatingPodMovement(Registry& registry, const system_context& context);
    void handleDetachedPodShooting(Registry& registry, system_context context);
    void handlePodToggle(Registry& registry);
    void updateDetachedPodPosition(Registry& registry, const system_context& context);
    void handlePlayerDamage(Registry& registry);
    void createPodLaserProjectile(Registry& registry, system_context context, transform_component_s pos, float angle,
                                  int damage);
    bool allPlayersHavePods(Registry& registry);
};
