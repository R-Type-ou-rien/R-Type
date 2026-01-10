#include "ServerInputManager.hpp"
#include "Components/NetworkComponents.hpp"

int ServerInputManager::updateActionFromPacket(ActionPacket packet, uint32_t client_id) {
    _client_states[client_id][packet.action_name] = packet.action_state;
    return 0;
}

const ActionState& ServerInputManager::getClientState(Action action, uint32_t client_id) const {
    auto client_it = _client_states.find(client_id);
    if (client_it == _client_states.end()) {
        return _default_state;
    }
    auto action_it = client_it->second.find(action);
    if (action_it == client_it->second.end()) {
        return _default_state;
    }
    return action_it->second;
}

bool ServerInputManager::isPressed(Action action, uint32_t client_id) const {
    return getClientState(action, client_id).pressed;
}

bool ServerInputManager::isJustPressed(Action action, uint32_t client_id) const {
    return getClientState(action, client_id).justPressed;
}

bool ServerInputManager::isJustReleased(Action action, uint32_t client_id) const {
    return getClientState(action, client_id).justReleased;
}

const ActionState& ServerInputManager::getState(Action action, uint32_t client_id) const {
    return getClientState(action, client_id);
}

void ServerInputManager::removeClient(uint32_t client_id) {
    _client_states.erase(client_id);
}