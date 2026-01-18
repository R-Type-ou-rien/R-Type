/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** EjectionSystem.cpp
*/

#pragma once

#include "EjectionSystem.hpp"
#include "Components/Velocity2DComponent.hpp"
#include <iostream>

void EjectionSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<EjectionComponent>();

    for (auto entity : entities) {
        auto& ejectionComp = registry.getComponent<EjectionComponent>(entity);
        if (!ejectionComp.ejected)
            continue;
        if (!registry.hasComponent<Velocity2D>(entity))
            continue;
        auto& velocity = registry.getComponent<Velocity2D>(entity);
        velocity.vx += ejectionComp.ejectionForce.x;
        velocity.vy += ejectionComp.ejectionForce.y;
        ejectionComp.ejected = true;
    }
}