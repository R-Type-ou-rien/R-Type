#include "ActionScriptSystem.hpp"

#include <cstddef>
#include <iostream>
#include <ostream>

#include "Components/StandardComponents.hpp"
#include "registry.hpp"

void ActionScriptSystem::update(Registry& registry, system_context context) {
    std::vector<size_t> entities = registry.getEntities<ActionScript>();
    for (auto entity : entities) {
        ActionScript& script = registry.getComponent<ActionScript>(entity);
        for (auto& [action_name, function] : script.actionOnPressed) {
            if (context.input.isJustPressed(action_name)) {
                std::cout << "Action " << action_name << " just pressed, callback !!";
                function(registry, context, entity);
            }
        }

        for (auto& [action_name, function] : script.actionPressed) {
            if (context.input.isPressed(action_name)) {
                std::cout << "Action " << action_name << " pressed, callback !!";
                function(registry, context, entity);
            }
        }

        for (auto& [action_name, function] : script.actionOnReleased) {
            if (context.input.isJustReleased(action_name)) {
                std::cout << "Action " << action_name << " just released, callback !!";
                function(registry, context, entity);
            }
        }
    }
    return;
}