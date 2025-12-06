/*
** TEST UNITAIRE : PHYSICS & DAMAGE
*/

#include <iostream>
#include <vector>
#include <string>

// Assure-toi que ces chemins sont bons
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
    Registry registry;
    
    // Instanciation des Systèmes
    BoxCollision boxSys;
    Damage damageSys; // Assure-toi que ta classe s'appelle bien DamageSystem dans damage.hpp
    HealthSystem healthSys;

    std::cout << "=== DEMARRAGE TEST COLLISION & DEGATS ===" << std::endl;

    // 1. Création du JOUEUR (Victime)
    Entity player = registry.createEntity();
    registry.addComponent(player, TransformComponent{100.0f, 100.0f});
    registry.addComponent(player, TeamComponent{TeamComponent::ALLY});
    registry.addComponent(player, HealthComponent{100, 100});
    registry.addComponent(player, BoxCollisionComponent{}); 
    registry.addComponent(player, sprite2D_component_s{{}, sf::IntRect(0, 0, 50, 50)});

    // 2. Création de l'ASTEROIDE (Attaquant)
    Entity asteroid = registry.createEntity();
    registry.addComponent(asteroid, TransformComponent{100.0f, 100.0f}); // DIRECTEMENT SUR LE JOUEUR
    registry.addComponent(asteroid, TeamComponent{TeamComponent::ENEMY});
    registry.addComponent(asteroid, DamageOnColision{50}); // Utilise DamageOnCollision (attention à l'orthographe Collision vs Colision selon ton fichier)
    registry.addComponent(asteroid, BoxCollisionComponent{});
    registry.addComponent(asteroid, sprite2D_component_s{{}, sf::IntRect(0, 0, 50, 50)});

    print_status(registry, player, "Player Avant");

    // Simulation d'une frame
    boxSys.update(registry, 0.0);
    damageSys.update(registry, 0.0);
    healthSys.update(registry, 0.0);
    
    print_status(registry, player, "Player Apres");

    return 0;
}