/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollSystem.hpp
*/

#pragma once

#include "../../common/Components/Components.hpp"
#include "../../common/ISystem.hpp"

class ScrollSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
