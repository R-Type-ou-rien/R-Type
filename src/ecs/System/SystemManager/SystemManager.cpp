#include "SystemManager.hpp"

void SystemManager::updateAll(float dt)
{
    for (auto& system : _systems) {
        system->update(_registry, dt);
    }
    return;
}
