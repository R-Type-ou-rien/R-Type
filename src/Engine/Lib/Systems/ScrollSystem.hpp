 

#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"

class ScrollSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
