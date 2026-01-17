/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** Sprite2D.hpp
*/

#pragma once

#include <vector>

#include <SFML/Graphics/Texture.hpp>

#include "../StructDatas/Rect2D.hpp"
#include "../StructDatas/RenderLayer.hpp"

#include "../../../Core/ECS/Utils/slot_map/slot_map.hpp"

struct Sprite2D {
    handle_t<sf::Texture> handle;
    struct Rect2D rect;
    RenderLayer layer = RenderLayer::Midground;
    bool flipX = false;
    bool flipY = false;
};
