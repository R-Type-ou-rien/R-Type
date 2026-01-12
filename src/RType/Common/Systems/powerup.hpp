#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/powerup_component.hpp"

class PowerUpSystem : public ISystem {
   public:
    PowerUpSystem() = default;
    ~PowerUpSystem() = default;
    void update(Registry& registry, system_context context) override;

    static void spawnSpeedUp(Registry& registry, system_context context, float x, float y);
    static void applyPowerUp(Registry& registry, Entity player, PowerUpComponent::PowerUpType type, float value,
                             float duration);

   private:
    void updateActivePowerUps(Registry& registry, system_context context);
    void checkPowerUpCollisions(Registry& registry, system_context context);
};
