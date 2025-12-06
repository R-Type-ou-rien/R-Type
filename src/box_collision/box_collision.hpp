#pragma once

#include <list>

#include "ecs/Registry/registry.hpp"
#include "tag_component/tag_component.hpp"
#include "transform_component/transform.hpp"

struct BoxCollisionComponent {
    TagComponent collision;
    std::vector<std::string> tagCollision;
};

class BoxCollision {
   public:
    BoxCollision() = default;
    ~BoxCollision() = default;
    void update(Registry& registry, double time_now);

   private:
    bool checkSize(const TransformComponent& a, const TransformComponent& b, sf::Vector2<int> size, sf::Vector2<int> size_b);
};
