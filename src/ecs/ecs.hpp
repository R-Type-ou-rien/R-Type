#include "Registry/registry.hpp"
#include "System/SystemManager.hpp"

class ECS {
    public:
        ECS() : systems(registry) {}

        ECS(unsigned int width = 800, unsigned int height = 600, const std::string& title = "R-Type")
        : _window(sf::VideoMode(width, height), title), 
        systems(registry) 
        {
            _window.setFramerateLimit(60);
        }

        void update(float dt) {
            systems.updateAll(dt);
        }

        void run() {
            sf::Clock clock;

            while (_window.isOpen()) {
                sf::Event event;
                while (_window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed)
                        _window.close();
                }
                sf::Time elapsed = clock.restart();
                float dt = elapsed.asSeconds();
                systems.updateAll(dt);
            }
        }

        sf::RenderWindow& getWindow() {
            return _window;
        }

        Registry& registry;
        SystemManager systems;

    private:
        sf::RenderWindow _window;
};