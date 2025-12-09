#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>

class WindowManager {
   private:
    sf::RenderWindow _window;

   public:
    WindowManager(unsigned int width, unsigned int height, const std::string& title);
    sf::RenderWindow& getWindow();
    bool isOpen();
    void clear();
    void display();
    std::optional<sf::Event> pollEvent();
};