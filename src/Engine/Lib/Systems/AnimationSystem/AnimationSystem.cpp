/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** AnimationSystem.cpp
*/

#include "AnimationSystem.hpp"
#include <algorithm>

void AnimationSystem::update(Registry& registry, system_context context) {
    const float dt = context.dt;

    const auto& animEntities = registry.getEntities<AnimatedSprite2D>();
    for (Entity e : animEntities) {
        auto& anim = registry.getComponent<AnimatedSprite2D>(e);

        if (!anim.playing)
            continue;

        auto it = anim.animations.find(anim.currentAnimation);
        if (it == anim.animations.end())
            continue;

        const AnimationClip& clip = it->second;
        if (clip.frames.empty())
            continue;

        if (anim.currentFrameIndex >= clip.frames.size())
            anim.currentFrameIndex = clip.frames.size() - 1;

        anim.timer += dt;

        const float frameDuration = (clip.frameDuration > 0.f) ? clip.frameDuration : 0.1f;

        while (anim.timer >= frameDuration && anim.playing) {
            anim.timer -= frameDuration;
            advanceFrame(anim, clip);
        }
    }
}

void AnimationSystem::advanceFrame(AnimatedSprite2D& anim, const AnimationClip& clip) {
    const std::size_t last = clip.frames.size() - 1;

    switch (clip.mode) {
        case AnimationMode::Loop: {
            if (anim.currentFrameIndex >= last)
                anim.currentFrameIndex = 0;
            else
                anim.currentFrameIndex++;
            break;
        }

        case AnimationMode::Once: {
            if (anim.currentFrameIndex >= last) {
                anim.currentFrameIndex = last;
                anim.playing = false;
            } else {
                anim.currentFrameIndex++;
            }
            break;
        }

        case AnimationMode::PingPong: {
            if (anim.loopDirection >= 0) {
                if (anim.currentFrameIndex >= last) {
                    anim.loopDirection = -1;
                    if (last > 0)
                        anim.currentFrameIndex--;
                } else {
                    anim.currentFrameIndex++;
                }
            } else {
                if (anim.currentFrameIndex == 0) {
                    anim.loopDirection = +1;
                    if (last > 0)
                        anim.currentFrameIndex++;
                } else {
                    anim.currentFrameIndex--;
                }
            }
            break;
        }
    }
}
