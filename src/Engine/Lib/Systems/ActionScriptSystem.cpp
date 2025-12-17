#include "ActionScriptSystem.hpp"

#include <cstddef>
#include <iostream>

#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"
#include "InputManager.hpp"
#include "registry.hpp"

void ActionScriptSystem::update(Registry& registry, system_context context) {
    if (!context.input.has_value()) {
        return;
    }
    InputManager input = context.input.value();

    std::vector<size_t> entities = registry.getEntities<ActionScript>();
    for (auto entity : entities) {
        
        if (context.client_id.has_value() && registry.hasComponent<NetworkIdentity>(entity)) {
            const NetworkIdentity& netId = registry.getComponent<NetworkIdentity>(entity);
            if (netId.owner_user_id != context.client_id.value().get()) {
                continue;  
            }
        }

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