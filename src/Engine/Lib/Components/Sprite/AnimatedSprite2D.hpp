/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** AnimatedSprite2D.hpp
*/

#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "../StructDatas/Rect2D.hpp"
#include "../StructDatas/RenderLayer.hpp"

#include "../../../Core/ECS/Utils/slot_map/slot_map.hpp"
#include "../../../Resources/ResourceConfig.hpp"

enum class AnimationMode { Loop, Once, PingPong };

struct AnimationClip {
    handle_t<TextureAsset> handle;
    std::vector<Rect2D> frames;
    float frameDuration = 0.1f;
    AnimationMode mode = AnimationMode::Loop;
};

struct AnimatedSprite2D {
    static constexpr auto name = "AnimatedSprite2DComponent";
    RenderLayer layer = RenderLayer::Midground;

    std::unordered_map<std::string, AnimationClip> animations;
    std::string currentAnimation;
    std::string previousAnimation;
    std::size_t currentFrameIndex = 0;

    int loopDirection = 1;
    float timer = 0.0f;
    bool playing = true;
    bool flipX = false;
    bool flipY = false;

    void play(const std::string& name, bool restart = true) {
        if (currentAnimation != name || restart) {
            currentAnimation = name;
            currentFrameIndex = 0;
            loopDirection = 1;
            timer = 0.f;
        }
        playing = true;
    }

    void stop() { playing = false; }

    bool isPlaying() const { return playing; }

    void playIfNotPlaying(const std::string& name) {
        if (!playing || currentAnimation != name)
            play(name);
    }
};
