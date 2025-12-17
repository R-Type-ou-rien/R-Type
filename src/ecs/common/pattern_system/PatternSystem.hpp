/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PatternSystem
*/

#ifndef PATTERNSYSTEM_HPP_
#define PATTERNSYSTEM_HPP_

#include <SFML/System/Vector2.hpp>
#include <vector>

#include "../Components/Components.hpp"
#include "../tag_component/tag_component.hpp"
#include "../../Engine/Core/ECS/ISystem.hpp"
#include "../../Engine/Core/ECS/Registry/registry.hpp"

struct PatternComponent {
    std::vector<sf::Vector2f> waypoints;
    int current_index = 0;
    float speed = 100.0f;
    bool loop = false;
    bool is_active = true;
};

class PatternSystem : public ISystem {
   public:
    PatternSystem() = default;
    ~PatternSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
};

#endif /* !PATTERNSYSTEM_HPP_ */