/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** Percentage.hpp
*/

#pragma once

#include <cstddef>

struct PercentageHealth
{
    std::size_t totalLifeRespawn;
    float percent = 0;
    float fatalLimite = 200;
};
