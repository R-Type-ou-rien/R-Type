#pragma once

#include "ISystem.hpp"

class DestructionSystem : public ISystem {
   public:
    DestructionSystem() = default;
    ~DestructionSystem() = default;

    void update(Registry& registry, system_context context) override;
};
