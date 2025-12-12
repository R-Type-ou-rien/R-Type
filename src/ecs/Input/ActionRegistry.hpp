/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ActionRegistry.hpp
*/

#include <string>
#include <unordered_set>
#include <vector>

#include "InputAction.hpp"

class ActionRegistry {
   public:
    const Action& registerAction(const std::string& name);

    std::vector<Action>& getActions(void) { return this->_actions; }

    bool exists(const std::string& name) const;

   private:
    std::vector<Action> _actions;
    std::unordered_set<std::string> _knownNamesSet;
};
