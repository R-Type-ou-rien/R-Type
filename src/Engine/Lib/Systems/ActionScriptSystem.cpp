#include "ActionScriptSystem.hpp"

#include <cstddef>
#include <iostream>
#include <ostream>

#include "Components/StandardComponents.hpp"
#include "registry.hpp"

void ActionScriptSystem::update(Registry& registry, system_context context) {
#if defined(SERVER_BUILD)
    auto entities = registry.getEntities<ActionScript>();

    for (auto entity : entities) {
        const ActionScript& script = registry.getConstComponent<ActionScript>(entity);

        if (registry.hasComponent<NetworkIdentity>(entity)) {
            // Script on a networked entity. Check its owner.
            uint32_t ownerId = registry.getConstComponent<NetworkIdentity>(entity).ownerId;
            if (ownerId != 0) { // It's owned by a player
                for (auto& [action_name, function] : script.actionOnPressed) {
                    if (context.input.isJustPressed(action_name, ownerId)) {
                        function(registry, context, entity);
                    }
                }
                for (auto& [action_name, function] : script.actionPressed) {
                    if (context.input.isPressed(action_name, ownerId)) {
                        function(registry, context, entity);
                    }
                }
                for (auto& [action_name, function] : script.actionOnReleased) {
                    if (context.input.isJustReleased(action_name, ownerId)) {
                        function(registry, context, entity);
                    }
                }
            }
        } else {
            // Script on a non-networked, server-side-only entity.
            // This could be a "global" script that reacts to any player's input.
            for (uint32_t client_id : context.active_clients) {
                for (auto& [action_name, function] : script.actionOnPressed) {
                    if (context.input.isJustPressed(action_name, client_id)) {
                        function(registry, context, entity);
                    }
                }
                for (auto& [action_name, function] : script.actionPressed) {
                    if (context.input.isPressed(action_name, client_id)) {
                        function(registry, context, entity);
                    }
                }
                for (auto& [action_name, function] : script.actionOnReleased) {
                    if (context.input.isJustReleased(action_name, client_id)) {
                        function(registry, context, entity);
                    }
                }
            }
        }
    }
#else // CLIENT_BUILD
    auto entities = registry.getEntities<ActionScript>();
    for (auto entity : entities) {
        const ActionScript& script = registry.getConstComponent<ActionScript>(entity);
        for (auto& [action_name, function] : script.actionOnPressed) {
            if (context.input.isJustPressed(action_name)) {
                function(registry, context, entity);
            }
        }
        for (auto& [action_name, function] : script.actionPressed) {
            if (context.input.isPressed(action_name)) {
                function(registry, context, entity);
            }
        }
        for (auto& [action_name, function] : script.actionOnReleased) {
            if (context.input.isJustReleased(action_name)) {
                function(registry, context, entity);
            }
        }
    }
#endif
}