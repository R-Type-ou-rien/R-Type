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

#include "Components/StandardComponents.hpp"
#include "../../Engine/Core/ECS/ISystem.hpp"
#include "../../Engine/Core/ECS/Registry/registry.hpp"

class PatternSystem : public ISystem {
   public:
    PatternSystem() = default;
    ~PatternSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
};

#endif /* !PATTERNSYSTEM_HPP_ */
