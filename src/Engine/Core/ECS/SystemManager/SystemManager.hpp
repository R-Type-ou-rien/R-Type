#pragma once

#include <memory>
#include <vector>
#include <utility>

#include "../ISystem.hpp"

class SystemManager {
   public:
    explicit SystemManager(Registry& registry) : _registry(registry) {}

    template <typename T, typename... Args>
    T* addSystem(Args&&... args) {
        auto sys = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = sys.get();
        _systems.push_back(std::move(sys));
        return ptr;
    }

    void updateAll(system_context context);

   private:
    Registry& _registry;
    std::vector<std::unique_ptr<ISystem>> _systems;
};
