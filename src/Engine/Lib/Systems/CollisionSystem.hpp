#pragma once

#include <vector>
#include <utility>
#include "Components/StandardComponents.hpp"
#include "Components/tag_component.hpp"
#include "ISystem.hpp"
#include "registry.hpp"

class BoxCollision : public ISystem {
   public:
    BoxCollision() = default;
    ~BoxCollision() = default;
    void update(Registry& registry, system_context context) override;

   private:
    bool checkSize(const transform_component_s a, const transform_component_s b, std::pair<float, float> size,
                   std::pair<float, float> size_b);
    bool hasTagToCollide(BoxCollisionComponent entity_a, const TagComponent& entity_b);
};
