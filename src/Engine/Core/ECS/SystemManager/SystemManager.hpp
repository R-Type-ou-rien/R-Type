/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SystemManager.hpp
*/

#pragma once

#include <memory>
#include <vector>

#include "../ISystem.hpp"

class SystemManager {
   public:
    SystemManager(Registry& registry) : _registry(registry) {}

    template <typename T, typename... Args>
    void addSystem(Args&&... args) {
        _systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    void updateAll(system_context context);

   private:
    Registry& _registry;
    std::vector<std::unique_ptr<ISystem>> _systems;
};