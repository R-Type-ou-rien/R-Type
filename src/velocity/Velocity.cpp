/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Velocity
*/

#include "Velocity.hpp"

namespace Ecs {

VelocitySystem::~VelocitySystem()
{
}

void VelocitySystem::UpdatePosition(Placement_t& placement, const Velocity_t& velocity, float dt)
{
    placement.pos_x += velocity.x * dt;
    placement.pos_y += velocity.y * dt;
}

}