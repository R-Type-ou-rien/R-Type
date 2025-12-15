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
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        
        if constexpr (requires { system->init(_registry); }) {
            system->init(_registry);
        }

        _systems.push_back(std::move(system));
    }

    void updateAll(system_context context);

   private:
    Registry& _registry;
    std::vector<std::unique_ptr<ISystem>> _systems;
};
