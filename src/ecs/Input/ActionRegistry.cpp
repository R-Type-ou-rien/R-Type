/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ActionRegistry.cpp
*/

#include "ActionRegistry.hpp"

const Action& ActionRegistry::registerAction(const std::string& name)
{
    if (_knownNamesSet.insert(name).second) {
        _actions.emplace_back(name);
    }
    for (const auto& a : _actions) {
        if (a == name)
            return a;
    }
    return _actions.front();
}

bool ActionRegistry::exists(const std::string& name) const
{
    return _knownNamesSet.find(name) != _knownNamesSet.end();
}