/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SpawnSystem.cpp
*/

#include "SpawnSystem.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <algorithm>
#include <cmath>
#include <random>

static float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }

void SpawnSystem::update(Registry& registry, system_context context) {
    const auto& entities = registry.getEntities<SpawnComponent>();
    if (entities.empty())
        ; // on continue pour le nettoyage hors-écran éventuel

    for (Entity e : entities) {
        auto& sp = registry.getComponent<SpawnComponent>(e);
        if (!sp.active)
            continue;
        sp.elapsed += context.dt;
        if (sp.elapsed >= sp.interval) {
            sp.elapsed -= sp.interval;  // récurrent
            spawn_one(registry, context, sp);
        }
    }

    // Nettoyage: détruire les ENEMY sortis complètement à gauche de l'écran
    const auto& tagged = registry.getEntities<TagComponent>();
    for (Entity e : tagged) {
        if (!registry.hasComponent<transform_component_s>(e) || !registry.hasComponent<sprite2D_component_s>(e))
            continue;
        const auto& tags = registry.getComponent<TagComponent>(e).tags;
        if (std::find(tags.begin(), tags.end(), std::string("ENEMY")) == tags.end())
            continue;

        const auto& tr = registry.getComponent<transform_component_s>(e);
        const auto& spr = registry.getComponent<sprite2D_component_s>(e);
        const float scaledW = spr.dimension.width * tr.scale_x;

        // Si le sprite est totalement hors de l'écran sur la gauche -> destroy
        if ((tr.x + scaledW) < 0.f) {
            std::cout << "[SpawnSystem] Mob détruit (hors écran gauche) x=" << tr.x << ", y=" << tr.y << std::endl;
            registry.destroyEntity(e);
        }
    }
}

void SpawnSystem::spawn_one(Registry& registry, system_context& context, const SpawnComponent& cfg) {
    Entity e = registry.createEntity();

    // Fenêtre pour bornes
    const auto winSize = context.window.getSize();
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    // RNG simple (thread_local pour éviter le re-seed)
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> coin(0.f, 1.f);

    // Taille à l'écran
    const float w = cfg.frame.width * cfg.scale_x;
    const float h = cfg.frame.height * cfg.scale_y;

    // Position Y aléatoire dans l'écran (clamp pour rester visible)
    std::uniform_real_distribution<float> distY(0.f, std::max(1.f, winH - h));
    float y = clampf(distY(rng), 0.f, std::max(0.f, winH - h));

    // Apparition forcée hors écran (à droite)
    float x = winW + 20.f;

    // Transform
    transform_component_s tr{ x, y };
    tr.scale_x = cfg.scale_x;
    tr.scale_y = cfg.scale_y;
    registry.addComponent<transform_component_s>(e, tr);

    // Velocity (vers la gauche)
    registry.addComponent<Velocity2D>(e, Velocity2D{cfg.speed_x, 0.f});

    // Tags: ENEMY + AI pour permettre la collision avec les projectiles alliés
    registry.addComponent<TagComponent>(e, TagComponent{{"ENEMY", "AI"}});

    // Sprite
    sprite2D_component_s sprite{};
    sprite.animation_speed = 0.f;
    sprite.current_animation_frame = 0;
    sprite.dimension = cfg.frame;

    if (context.texture_manager.is_loaded(cfg.sprite_path)) {
        sprite.handle = context.texture_manager.get_handle(cfg.sprite_path).value();
    } else {
        sprite.handle = context.texture_manager.load_resource(cfg.sprite_path, sf::Texture(cfg.sprite_path));
    }
    registry.addComponent<sprite2D_component_s>(e, sprite);

    // Box collision pour permettre aux projectiles de toucher le mob
    registry.addComponent<BoxCollisionComponent>(e, BoxCollisionComponent{});
}
