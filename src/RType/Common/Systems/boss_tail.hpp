#pragma once

#include "ISystem.hpp"

class BossTailSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;
};
