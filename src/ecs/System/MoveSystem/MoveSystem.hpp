/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MoveSystem.hpp
*/

#pragma once

#include "../ISystem.hpp"
#include "../../Components/Components.hpp"

class MoveSystem : public ISystem {
    public:
        void init(Registry& registry) override {}

        void update(Registry& registry, float dt) override;
};