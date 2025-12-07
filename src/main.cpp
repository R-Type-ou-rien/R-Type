#include "ecs/ECS.hpp"
#include "ecs/Components/Components.hpp"
#include "ecs/System/ISystem.hpp"
#include "ecs/System/MoveSystem/MoveSystem.hpp"
#include "ecs/System/RenderSystem/RenderSystem.hpp"
#include "ecs/System/InputSystem/InputSystem.hpp"
#include "ecs/System/ShootSystem/ShootSystem.hpp"
#include "ecs/System/ProjectileSystem/ProjectileSystem.hpp"
#include "ecs/System/CooldownSystem/CooldownSystem.hpp"
#include "ecs/System/GamepadInputSystem/GamepadInputSystem.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

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


int main() {
    /* Instance of ECS class */
    ECS ecs(1920, 1080, "R-Type");

    /* An instance of a ressource manager */
    ResourceManager<sf::Texture> texture_manager;

    /* context instance, a struct containing what systems need  */
    system_context context = {0, texture_manager};

    /* system addition */
    ecs.systems.addSystem<InputSystem>();
    ecs.systems.addSystem<GamepadInputSystem>();
    ecs.systems.addSystem<ShootSystem>();
    ecs.systems.addSystem<MoveSystem>();
    ecs.systems.addSystem<ProjectileSystem>();
    ecs.systems.addSystem<RenderSystem>(ecs.getWindow());
    ecs.systems.addSystem<CooldownSystem>(ecs.getWindow());

    /* creation of an entity */
    Entity player = ecs.registry.createEntity();

    /* link of the entity with components */
    ecs.registry.addComponent(player, transform_component_s{100.f, 300.f});
    ecs.registry.addComponent(player, Velocity2D{0.f, 0.f});

    // contrôle manette
    ecs.registry.addComponent<GamepadControl>(player, {
        0,                      // joystickId (0 = première manette)
        sf::Joystick::Axis::X,        // axe horizontal
        sf::Joystick::Axis::Y,        // axe vertical
        XBOXA,                      // buttonShoot (bouton 0 = A sur Xbox, par ex.)
        300.f,                  // speed
        15.f                    // deadZone
    });

    // logique de tir
    ecs.registry.addComponent<Shooter>(player, {
        sf::Keyboard::Key::J,  // shootKey (toujours dispo au clavier)
        800.f,            // projectileSpeed
        2.0f,             // projectileLifetime
        3.0f,            // fireRate
        // 100.f               // timeSinceLastShot
    });

    /* 
        handle instance, a handle is a 'ticket' to ask to access to a ressource
        when inserting a ressource, the slot map returns a "ticket"
    */
    handle_t<sf::Texture> handle = texture_manager.load_resource("content/sprites/r-typesheet42.gif", sf::Texture("content/sprites/r-typesheet42.gif"));

    /*
        instance of a 2D sprite component
        Look !! it takes a "ticket" to access the texture later !!
    */
    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0.5f;
    sprite_info.current_animation_frame = 0;
    sprite_info.dimension.position = {0, 0};
    sprite_info.dimension.size =  {32, 16};
    sprite_info.z_index = 1;

    /* the component is linked to the player entity */
    ecs.registry.addComponent<sprite2D_component_s>(player, sprite_info);

    /*
        this part is simple sfml and will disapear but it will stay until I change the architecture to test
        you can still notice some changes like the event handing
    */
    sf::RenderWindow& window = ecs.getWindow();

    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        sf::Time elapsed = clock.restart();
        context.dt = elapsed.asSeconds();
        window.clear();
        ecs.update(context);
        window.display();
    }

    // ecs.run();
    return 0;
}
