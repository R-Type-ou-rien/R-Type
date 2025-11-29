/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ProjectileSystem.hpp
*/

#pragma once

#include <vector>
#include "../ISystem.hpp"
#include "../../Components/Components.hpp"

class ProjectileSystem : public ISystem {
    public:
        void init(Registry& registry) override {}

        void update(Registry& registry, float dt) override;
};
