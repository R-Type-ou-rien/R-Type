 

#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

class HUDSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
