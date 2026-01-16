#pragma once

#include <memory>
#include <vector>
#include <utility>

#include "../ISystem.hpp"

class SystemManager {
   public:
    explicit SystemManager(Registry& registry) : _registry(registry) {}

    template <typename T, typename... Args>
    void addSystem(Args&&... args) {
        _systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template <typename T>
    T* getSystem() {
        for (auto& system : _systems) {
            T* casted = dynamic_cast<T*>(system.get());
            if (casted) {
                return casted;
            }
        }
        return nullptr;
    }

    void updateAll(system_context context);

   private:
    Registry& _registry;
    std::vector<std::unique_ptr<ISystem>> _systems;
};
