#include <string>
#include <utility>
#include "DynamicActor.hpp"
#include "Components/StandardComponents.hpp"
#include "../../../../Common/Components/team_component.hpp"
#include "../../../../Common/Components/shooter.hpp"

#pragma once

class AI : public DynamicActor {
   public:
    AI(ECS& ecs, ResourceManager<TextureAsset>& textures, std::pair<float, float> pos);

    void setProjectileType(ShooterComponent::ProjectileType type);

    ShooterComponent::ProjectileType getProjectileType();

    void setShootingState(bool state);

    bool isShooting();

    void setFireRate(double fire_rate);

    double getFireRate();

    void setTeam(TeamComponent::Team team);

    TeamComponent::Team getTeam();

    void setLifePoint(int lifePoint);

    int getCurrentHealth();

    int getMaxHealth();

    void setCurrentHealth(int health);

    void takeDamage(int damage);
};
