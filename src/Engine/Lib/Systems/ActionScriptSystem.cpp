#include "ActionScriptSystem.hpp"

#include <cstddef>
#include <iostream>

#include "Components/StandardComponents.hpp"
#include "InputManager.hpp"
#include "registry.hpp"

void ActionScriptSystem::update(Registry& registry, system_context context) {
    if (!context.input.has_value()) {
        throw std::logic_error("The input manager is not initalized in the given context");
    }
    InputManager input = context.input.value();

    std::vector<size_t> entities = registry.getEntities<ActionScript>();
    for (auto entity : entities) {
        ActionScript script = registry.getComponent<ActionScript>(entity);

        for (auto [action_name, function] : script.actionOnPressed) {
            if (input.isJustPressed(action_name)) {
                function(registry, context, entity);
            }
        }

        for (auto [action_name, function] : script.actionPressed) {
            if (input.isPressed(action_name)) {
                function(registry, context, entity);
            }
        }

        for (auto [action_name, function] : script.actionOnReleased) {
            if (input.isJustReleased(action_name)) {
                function(registry, context, entity);
            }
        }
    }
    return;
}