/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MoveSystem.hpp
*/

#pragma once

#include "../../common/Components/Components.hpp"
#include "../../common/ISystem.hpp"

class MoveSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};