#include "SystemManager.hpp"

void SystemManager::updateAll(float dt)
{
    for (auto& system : _systems) {
        system->update(_registry, dt);
    }
    return;
}

template <typename T, typename... Args>
void SystemManager::addSystem(Args&&... args) {
    _systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    _systems.back()->init(_registry);
}