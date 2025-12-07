#include "SystemManager.hpp"

#include "ecs/System/ISystem.hpp"

void SystemManager::updateAll(system_context context) {
    for (auto& system : _systems) {
        system->update(_registry, context);
    }
    return;
}
