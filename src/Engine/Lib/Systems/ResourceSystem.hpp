#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"

class ResourceSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context);
};