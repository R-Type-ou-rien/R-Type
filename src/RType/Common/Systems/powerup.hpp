#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/powerup_component.hpp"
#include <unordered_map>
#include <vector>
#include <functional>

class PowerUpSystem : public ISystem {
   public:
    PowerUpSystem();
    ~PowerUpSystem() = default;
    void update(Registry& registry, system_context context) override;

    static void spawnSpeedUp(Registry& registry, system_context context, float x, float y);

   private:
    using PowerUpApplicator = std::function<void(Registry&, Entity, float, float)>;
    using PowerUpDeactivator = std::function<void(Registry&, Entity)>;

    std::unordered_map<PowerUpComponent::PowerUpType, PowerUpApplicator> apply_power_up;
    std::unordered_map<PowerUpComponent::PowerUpType, PowerUpDeactivator> deactivate_power_up;

    void initialize_power_up_maps();
    void applyPowerUp(Registry& registry, Entity player, PowerUpComponent::PowerUpType type, float value,
                      float duration);
    void updateActivePowerUps(Registry& registry, system_context context);
    void checkPowerUpCollisions(Registry& registry, system_context context);
};
