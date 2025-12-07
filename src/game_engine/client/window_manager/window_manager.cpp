#include "window_manager.hpp"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <optional>

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
{
    _window.create(sf::VideoMode{{height, width}}, title);
    _window.setFramerateLimit(60);
}

sf::RenderWindow& WindowManager::getWindow()
{
    return _window;
}

bool WindowManager::isOpen()
{
    return _window.isOpen();
}

void WindowManager::clear()
{
    _window.clear();
    return;
}

void WindowManager::display()
{
    _window.display();
    return;
}

void WindowManager::handleEvent(std::optional<sf::Event>& event)
{
    while (std::optional catch_event = _window.pollEvent()) {
        if (catch_event->is<sf::Event::Closed>())
            _window.close();
        event = catch_event;
    }
    return;
}
