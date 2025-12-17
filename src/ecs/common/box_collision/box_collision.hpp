#pragma once

#include <string>
#include <vector>

#include "../Components/Components.hpp"
#include "../tag_component/tag_component.hpp"
#include "../../Engine/Core/ECS/ISystem.hpp"
#include "../../Engine/Core/ECS/Registry/registry.hpp"

struct BoxCollisionComponent {
    TagComponent collision;
    std::vector<std::string> tagCollision;
};

class BoxCollision : public ISystem {
   public:
    BoxCollision() = default;
    ~BoxCollision() = default;
    void update(Registry& registry, system_context context) override;

   private:
    bool checkSize(const transform_component_s& a, const transform_component_s& b, sf::Vector2<int> size,
                   sf::Vector2<int> size_b);
};
