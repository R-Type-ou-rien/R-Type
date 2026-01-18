/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SystemManager.cpp
*/

#include "SystemManager.hpp"

void SystemManager::updateAll(system_context context) {
    for (auto& system : _systems) {
        system->update(_registry, context);
    }
}
