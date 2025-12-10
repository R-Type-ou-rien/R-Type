/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PatternSystem
*/

#ifndef PATTERNSYSTEM_HPP_
#define PATTERNSYSTEM_HPP_

#include "../Components/Components.hpp"
#include "../tag_component/tag_component.hpp"
#include "ecs/common/ISystem.hpp"
#include "ecs/common/Registry/registry.hpp"

struct Pattern {
    enum PatternType { STRAIGHT, LEFTRIGHT, CIRCLE, WALK };
    PatternType type;
    float speed;
    float amplitude;
    float frequency;
    bool is_moving;
};

class PatternSystem : public ISystem {
   public:
    PatternSystem() = default;
    ~PatternSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
};

#endif /* !PATTERNSYSTEM_HPP_ */
