#include "WindowManager.hpp"

#include <optional>
#include <string>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title) {
    _window.create(sf::VideoMode{{width, height}}, title);
    _window.setFramerateLimit(60);
}

sf::RenderWindow& WindowManager::getWindow() {
    return _window;
}

bool WindowManager::isOpen() {
    return _window.isOpen();
}

void WindowManager::clear() {
    _window.clear();
    return;
}

void WindowManager::display() {
    _window.display();
    return;
}

std::optional<sf::Event> WindowManager::pollEvent() {
    return _window.pollEvent();
}
