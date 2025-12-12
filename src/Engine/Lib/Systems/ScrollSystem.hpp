/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScrollSystem.hpp
*/

#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"

class ScrollSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
