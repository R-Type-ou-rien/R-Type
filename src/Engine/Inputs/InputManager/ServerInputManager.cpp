#include "ServerInputManager.hpp"
#include "Components/NetworkComponents.hpp"

int ServerInputManager::updateActionFromPacket(ActionPacket packet)
{
    if (_states.find(packet.action_name) != _states.end()) {
        _states[packet.action_name] = packet.action_state;
        return 0;
    }
    return -1;
}