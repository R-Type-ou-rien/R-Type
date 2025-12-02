#pragma one

#include "shoot_feature/shooter.hpp"
#include "shooter.hpp"

namespace ECS {
void ShooterSystem::update(Registry& registry) {}
void ShooterSystem::handle_shot(Registry& registry) {}
void ShooterSystem::check_pod_shoot(Registry& registry) {}
void ShooterSystem::create_projectile(Type type, Team team, Position pos) {}

}  // namespace ECS
