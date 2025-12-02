/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISystem.hpp
*/

#pragma once

#include "../Registry.hpp"

class ISystem {
    public:
        virtual ~ISystem() = default;

        virtual void init(Registry& registry) = 0;

        virtual void update(Registry& registry, float dt) = 0;
};