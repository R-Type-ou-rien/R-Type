#pragma once

#include "ISystem.hpp"

class BackgroundSystem : public ISystem {
   public:
    BackgroundSystem() = default;
    void update(Registry& registry, system_context context) override;
};
