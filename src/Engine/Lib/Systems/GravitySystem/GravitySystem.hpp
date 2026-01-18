/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** GravitySystem.hpp
*/

#include "../../../Core/ECS/ISystem.hpp"
#include "../../Components/GravityComponent.hpp"

class GravitySystem : public ISystem {
   public:
    GravitySystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void checkGrounded(Registry& registry, Entity entity, system_context context);
};