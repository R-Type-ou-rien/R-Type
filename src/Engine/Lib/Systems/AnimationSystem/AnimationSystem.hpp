/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** AnimationSystem.hpp
*/

#pragma once

#include "../../Lib/Components/Sprite/AnimatedSprite2D.hpp"
#include "ISystem.hpp"

class AnimationSystem : public ISystem {
   public:
    AnimationSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    static void advanceFrame(AnimatedSprite2D& anim, const AnimationClip& clip);
};
