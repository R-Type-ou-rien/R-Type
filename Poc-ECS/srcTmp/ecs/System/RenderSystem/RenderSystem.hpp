/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RenderSystem.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <iostream>
#include "../ISystem.hpp"
#include "../Components/Components.hpp"

class RenderSystem : public ISystem {
    public:
        RenderSystem(sf::RenderWindow& window) : _window(window) {}

        void init(Registry& registry) override {
        }

        void update(Registry& registry, float dt) override;

    private:
        sf::RenderWindow& _window;
        
        std::unordered_map<std::string, sf::Texture> _textureCache;

        sf::Texture& getTexture(const std::string& path);

        void drawEntity(const Position2D& pos, const Sprite2D& spriteData);
};