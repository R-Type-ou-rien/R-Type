/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** ProjectileComponent.hpp
*/

#pragma once

#include "../../../Engine/Core/ECS/EcsType.hpp"

struct SmashProjectileComponent {
    static constexpr auto name = "SmashProjectileComponent";
    Entity ownerId;
    float damage;
    float lifetime;
};
