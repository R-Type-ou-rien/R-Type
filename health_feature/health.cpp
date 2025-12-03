#pragma once

#include "health_feature/health.hpp"

#include <iostream>
#include "health.hpp"

namespace ECS {
void HealthSystem::update(Registry& registry) {}
void HealthSystem::check_death(Registry& registry){} //, Entity e) {}
void HealthSystem::handle_enemy_death(Registry& registry){} //, Entity e) {}
void HealthSystem::handle_player_death(Registry& registry){} //, Entity e) {}
}  // namespace ECS
