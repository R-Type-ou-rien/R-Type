#include "ecs/ECS.hpp"
#include "ecs/Components/Components.hpp"
#include "ecs/System/MoveSystem/MoveSystem.hpp"
#include "ecs/System/RenderSystem/RenderSystem.hpp"
#include "ecs/System/InputSystem/InputSystem.hpp"
#include "ecs/System/ShootSystem/ShootSystem.hpp"
#include "ecs/System/ProjectileSystem/ProjectileSystem.hpp"
#include "ecs/System/CooldownSystem/CooldownSystem.hpp"
#include "ecs/System/GamepadInputSystem/GamepadInputSystem.hpp"
#include <iostream>

#define XBOXA 0
#define XBOXB 1
#define XBOXX 2
#define XBOXY 3

#define XBOXLB 4
#define XBOXRB 5
#define XBOXSYSL 6
#define XBOXSYSR 7
#define XBOXMAIN 8
#define XBOXJOYL 9
#define XBOXJOYR 10


#include <iostream>
#include <ostream>
int main() {
    ECS ecs(1920, 1080, "R-Type");

    ecs.systems.addSystem<InputSystem>();
    ecs.systems.addSystem<GamepadInputSystem>();
    ecs.systems.addSystem<ShootSystem>();
    ecs.systems.addSystem<MoveSystem>();
    ecs.systems.addSystem<ProjectileSystem>();
    ecs.systems.addSystem<RenderSystem>(ecs.getWindow());
    ecs.systems.addSystem<CooldownSystem>(ecs.getWindow());

    Entity player = ecs.registry.createEntity();

    ecs.registry.addComponent(player, Position2D{100.f, 300.f});
    ecs.registry.addComponent(player, Velocity2D{0.f, 0.f});

    // contrôle manette
    ecs.registry.addComponent(player, GamepadControl{
        0,                      // joystickId (0 = première manette)
        sf::Joystick::X,        // axe horizontal
        sf::Joystick::Y,        // axe vertical
        XBOXA,                      // buttonShoot (bouton 0 = A sur Xbox, par ex.)
        300.f,                  // speed
        15.f                    // deadZone
    });

    // logique de tir
    ecs.registry.addComponent(player, Shooter{
        sf::Keyboard::J,  // shootKey (toujours dispo au clavier)
        800.f,            // projectileSpeed
        2.0f,             // projectileLifetime
        3.0f,            // fireRate
        // 100.f               // timeSinceLastShot
    });

    ecs.registry.addComponent(player, Sprite2D{
        "content/sprites/r-typesheet42.gif",
        0, 0, 32, 16, 3.0f
    });

    sf::RenderWindow& window = ecs.getWindow();

    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
        window.clear();
        ecs.update(dt);
        window.display();
    }

    // ecs.run();
    return 0;
}
