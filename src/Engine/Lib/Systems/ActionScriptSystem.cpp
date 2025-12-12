#include "ActionScriptSystem.hpp"

#include <cstddef>

#include "Components/StandardComponents.hpp"
#include "registry.hpp"

void ActionScriptSystem::update(Registry& registry, system_context context) {
    std::vector<size_t> entities = registry.getEntities<ActionScript>();

    for (auto entity : entities) {
        ActionScript script = registry.getComponent<ActionScript>(entity);

        for (auto [action_name, function] : script.actionOnPressed) {
            if (context.input.isJustPressed(action_name))
                function(registry, context, entity);
        }

        for (auto [action_name, function] : script.actionPressed) {
            if (context.input.isPressed(action_name))
                function(registry, context, entity);
        }

        for (auto [action_name, function] : script.actionOnReleased) {
            if (context.input.isJustReleased(action_name))
                function(registry, context, entity);
        }
    }
    return;
}