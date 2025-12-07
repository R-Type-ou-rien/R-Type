/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollSystem.hpp
*/

#pragma once

#include "../../Components/Components.hpp"
#include "../ISystem.hpp"

class ScrollSystem : public ISystem {
   public:
    void init(Registry& registry) override {}

    void update(Registry& registry, system_context context) override;
};
