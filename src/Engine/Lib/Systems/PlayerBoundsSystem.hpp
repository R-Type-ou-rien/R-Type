#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "Components/StandardComponents.hpp"
#include <cmath>

// a changer pour qu'il s'adapte a la taille de l'ecran
struct WorldBoundsComponent {
    static constexpr auto name = "WorldBoundsComponent";
    float min_x = 0.0f;
    float max_x = 1920.0f;
    float min_y = 0.0f;
    float max_y = 1080.0f;
};


class PlayerBoundsSystem : public ISystem {
   public:
    PlayerBoundsSystem() = default;
    ~PlayerBoundsSystem() = default;
    void update(Registry& registry, system_context context) override;
};
