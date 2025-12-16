#include "Projectile.hpp"

ProjectileActor::ProjectileActor(ECS& ecs, std::pair<float, float> pos, const std::string name)
    : DynamicActor(ecs, false, name) {}