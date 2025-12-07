#pragma once

#include "ecs/Registry/registry.hpp"
#include "ecs/System/ISystem.hpp"
#include "tag_component/tag_component.hpp"
#include "transform_component/transform.hpp"

struct BoxCollisionComponent {
    TagComponent collision;
    std::vector<std::string> tagCollision;
};

class BoxCollision : public ISystem {
   public:
    BoxCollision() = default;
    ~BoxCollision() = default;
    void init(Registry& registry) override {};
    void update(Registry& registry, system_context context) override;

   private:
    bool checkSize(const TransformComponent& a, const TransformComponent& b, sf::Vector2<int> size, sf::Vector2<int> size_b);
};
