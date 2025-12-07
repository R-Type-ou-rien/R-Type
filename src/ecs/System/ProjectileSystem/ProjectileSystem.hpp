/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ProjectileSystem.hpp
*/

#pragma once

#include <vector>

#include "../../Components/Components.hpp"
#include "../ISystem.hpp"

class ProjectileSystem : public ISystem {
   public:
    void init(Registry& registry) override {}

    void update(Registry& registry, system_context context) override;
};
