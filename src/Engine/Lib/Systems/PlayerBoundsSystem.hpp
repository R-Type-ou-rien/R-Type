#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "Components/StandardComponents.hpp"
#include <cmath>

struct WorldBoundsComponent {
    static constexpr auto name = "WorldBoundsComponent";
    float min_x = 0.0f;
    float min_y = 0.0f;
};


class PlayerBoundsSystem : public ISystem {
   public:
    PlayerBoundsSystem() = default;
    ~PlayerBoundsSystem() = default;
    void update(Registry& registry, system_context context) override;
};
