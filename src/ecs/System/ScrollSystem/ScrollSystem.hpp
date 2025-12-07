/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollSystem.hpp
*/

#pragma once

#include "../ISystem.hpp"
#include "../../Components/Components.hpp"

class ScrollSystem : public ISystem {
    public:
        void init(Registry& registry) override {}

        void update(Registry& registry, system_context context) override;
};
