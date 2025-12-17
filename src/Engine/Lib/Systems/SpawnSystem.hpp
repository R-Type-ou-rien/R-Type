 

#pragma once

#include <random>
#include <string>

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"

class SpawnSystem : public ISystem {
   public:
    SpawnSystem() = default;
    ~SpawnSystem() override = default;

    void update(Registry& registry, system_context context) override;
};
