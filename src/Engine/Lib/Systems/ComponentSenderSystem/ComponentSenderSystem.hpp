#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

class ComponentSenderSystem : public ISystem {
   public:
    ComponentSenderSystem() = default;
    ~ComponentSenderSystem() = default;
    void update(Registry& ref, system_context ctx);
};