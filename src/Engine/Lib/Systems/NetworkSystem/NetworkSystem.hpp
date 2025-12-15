#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include <functional>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <typeindex>

class NetworkSystem : public ISystem {
    public:
        NetworkSystem() = default;
        ~NetworkSystem() = default;
        void update(Registry& registry, system_context context) override;
        
};