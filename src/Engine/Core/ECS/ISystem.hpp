/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ISystem.hpp
*/

#pragma once

#include "Registry/registry.hpp"
#include "Context.hpp"

class ISystem {
   public:
    virtual ~ISystem() = default;

    virtual void update(Registry& registry, system_context context) = 0;
};
