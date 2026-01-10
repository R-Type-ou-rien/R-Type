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
    void handlePodCollection(Registry& registry, system_context context);
    void updateAttachedPodPosition(Registry& registry, system_context context);
    void updateFloatingPodMovement(Registry& registry, system_context context);
    void handleDetachedPodShooting(Registry& registry, system_context context);
    void handlePodToggle(Registry& registry, system_context context);
    void updateDetachedPodPosition(Registry& registry, system_context context);
    void handlePlayerDamage(Registry& registry, system_context context);
    void createPodLaserProjectile(Registry& registry, system_context context, transform_component_s pos, float angle,
                                  int damage);
    bool allPlayersHavePods(Registry& registry);
};
