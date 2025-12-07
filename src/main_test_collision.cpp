/*
** TEST UNITAIRE : PHYSICS & DAMAGE
*/

#include <iostream>
#include <string>

// Assure-toi que ces chemins sont bons
#include "ecs/Components/Components.hpp"
#include "ecs/ECS.hpp"
#include "ecs/Registry/registry.hpp"
#include "ecs/Components/Components.hpp"
#include "transform_component/transform.hpp"
#include "team_component/team_component.hpp"
#include "health_feature/health.hpp"
#include "box_collision/box_collision.hpp"
#include "damage_feature/damage.hpp"

// Fonction utilitaire pour afficher les PV
void print_status(Registry& reg, Entity e, std::string name) {
    if (reg.hasComponent<HealthComponent>(e)) {
        auto& hp = reg.getComponent<HealthComponent>(e);
        std::cout << "STATUS [" << name << "] ID: " << e 
                  << " | PV: " << hp.current_hp << "/" << hp.max_hp << std::endl;
    } else {
        std::cout << "STATUS [" << name << "] ID: " << e << " est MORT." << std::endl;
    }
}

int main() {
    ECS ecs;
    SlotMap<sf::Texture> texture_manager;
    system_context context = {0, texture_manager};
    
    // Instanciation des Systèmes

    std::cout << "=== DEMARRAGE TEST COLLISION & DEGATS ===" << std::endl;

    ecs.systems.addSystem<BoxCollision>();
    ecs.systems.addSystem<Damage>();
    ecs.systems.addSystem<HealthSystem>();

    // 1. Création du JOUEUR (Victime)
    Entity player = ecs.registry.createEntity();
    handle_t<sf::Texture> handle = texture_manager.insert(sf::Texture("content/sprites/r-typesheet42.gif"));
    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0.5f;
    sprite_info.current_animation_frame = 0;
    sprite_info.dimension.position = {0, 0};
    sprite_info.dimension.size =  {32, 16};
    sprite_info.z_index = 1;
    ecs.registry.addComponent(player, TransformComponent{100.0f, 100.0f});
    ecs.registry.addComponent(player, TeamComponent{TeamComponent::ALLY});
    ecs.registry.addComponent(player, HealthComponent{100, 100});
    ecs.registry.addComponent(player, BoxCollisionComponent{});
    ecs.registry.addComponent<sprite2D_component_s>(player, sprite_info);

    // 2. Création de l'ASTEROIDE (Attaquant)
    Entity asteroid = ecs.registry.createEntity();
    ecs.registry.addComponent(asteroid, TransformComponent{100.0f, 100.0f}); // DIRECTEMENT SUR LE JOUEUR
    ecs.registry.addComponent(asteroid, TeamComponent{TeamComponent::ENEMY});
    ecs.registry.addComponent(asteroid, DamageOnColision{50}); // Utilise DamageOnCollision (attention à l'orthographe Collision vs Colision selon ton fichier)
    ecs.registry.addComponent(asteroid, BoxCollisionComponent{});
    ecs.registry.addComponent<sprite2D_component_s>(asteroid, sprite_info);

    print_status(ecs.registry, player, "Player Avant");

    // Simulation d'une frame
    ecs.update(context);
    print_status(ecs.registry, player, "Component update");
    
    print_status(ecs.registry, player, "Player Apres");

    return 0;
}