/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** EjectionSystem.hpp
*/

#pragma once

#include "../../Core/ECS/ISystem.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/StructDatas/Vector2D.hpp"

struct EjectionComponent
{
    Vector2D ejectionForce;
    float duration;
    bool ejected = false;
};

class EjectionSystem : public ISystem {
public:
    void update(Registry& registry, system_context context) override;

};
