/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GravityComponent.hpp
*/

#pragma once

#include "StructDatas/Vector2D.hpp"

struct GravityComponent
{
    static constexpr auto name = "GravityComponent";
    float force = 10.0f;
    float vectorY = 0.0f;
    bool grounded = false;
};
