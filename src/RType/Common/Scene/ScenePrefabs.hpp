/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ScenePrefabs - Register game prefabs for scene loading
*/

#pragma once

#include <string>
#include <unordered_map>
#include "src/Engine/Core/Scene/SceneManager.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/terrain_component.hpp"
#include "../Components/team_component.hpp"
#include "../Systems/health.hpp"
#include "../Systems/shooter.hpp"
#include "../Systems/ai_behavior.hpp"
#include "../Systems/score.hpp"

class ScenePrefabs {
   public:
    static void registerAll(SceneManager& scene_manager, ResourceManager<TextureAsset>& texture_manager) {
        registerWallPrefab(scene_manager, texture_manager);
        registerTurretPrefab(scene_manager, texture_manager);
    }

   private:
    static void registerWallPrefab(SceneManager& scene_manager, ResourceManager<TextureAsset>& texture_manager) {
        scene_manager.registerPrefab(
            "Wall", [&texture_manager](Registry& registry, Entity entity,
                                       const std::unordered_map<std::string, std::any>& props) {
                float x = 0, y = 0, width = 64, height = 64;
                std::string sprite_path = "src/RType/Common/content/sprites/r-typesheet14.gif";
                bool destructible = false;

                if (props.count("x"))
                    x = std::any_cast<float>(props.at("x"));
                if (props.count("y"))
                    y = std::any_cast<float>(props.at("y"));
                if (props.count("width"))
                    width = std::any_cast<float>(props.at("width"));
                if (props.count("height"))
                    height = std::any_cast<float>(props.at("height"));
                if (props.count("sprite"))
                    sprite_path = std::any_cast<std::string>(props.at("sprite"));
                if (props.count("destructible"))
                    destructible = std::any_cast<bool>(props.at("destructible"));

                registry.addComponent<transform_component_s>(entity, {x, y});
                registry.addComponent<Velocity2D>(entity, {-100.0f, 0.0f});

                WallComponent wall;
                wall.is_destructible = destructible;
                wall.blocks_projectiles = true;
                wall.blocks_player = true;
                wall.blocks_enemies = true;
                wall.width = width;
                wall.height = height;
                registry.addComponent<WallComponent>(entity, wall);

                if (destructible) {
                    registry.addComponent<HealthComponent>(entity, {200, 200, 0.0f, 0.0f});
                }

                handle_t<TextureAsset> handle = texture_manager.load(sprite_path, TextureAsset(sprite_path));

                sprite2D_component_s sprite;
                sprite.handle = handle;
                sprite.dimension = {0, 0, 32, 32};
                sprite.z_index = 3;
                registry.addComponent<sprite2D_component_s>(entity, sprite);

                auto& transform = registry.getComponent<transform_component_s>(entity);
                transform.scale_x = width / 32.0f;
                transform.scale_y = height / 32.0f;

                BoxCollisionComponent collision;
                collision.tagCollision.push_back("PLAYER");
                collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
                registry.addComponent<BoxCollisionComponent>(entity, collision);

                TagComponent tags;
                tags.tags.push_back("WALL");
                tags.tags.push_back("TERRAIN");
                registry.addComponent<TagComponent>(entity, tags);
            });
    }

    static void registerTurretPrefab(SceneManager& scene_manager, ResourceManager<TextureAsset>& texture_manager) {
        scene_manager.registerPrefab(
            "Turret", [&texture_manager](Registry& registry, Entity entity,
                                         const std::unordered_map<std::string, std::any>& props) {
                float x = 0, y = 0;
                float fire_rate = 1.5f;
                std::string shoot_pattern = "STRAIGHT";
                std::string sprite_path = "src/RType/Common/content/sprites/r-typesheet13.gif";

                if (props.count("x"))
                    x = std::any_cast<float>(props.at("x"));
                if (props.count("y"))
                    y = std::any_cast<float>(props.at("y"));
                if (props.count("fire_rate"))
                    fire_rate = std::any_cast<float>(props.at("fire_rate"));
                if (props.count("shoot_pattern"))
                    shoot_pattern = std::any_cast<std::string>(props.at("shoot_pattern"));
                if (props.count("sprite"))
                    sprite_path = std::any_cast<std::string>(props.at("sprite"));

                registry.addComponent<transform_component_s>(entity, {x, y});
                registry.addComponent<Velocity2D>(entity, {-100.0f, 0.0f});

                registry.addComponent<HealthComponent>(entity, {80, 80, 0.0f, 0.5f});
                registry.addComponent<TeamComponent>(entity, {TeamComponent::ENEMY});
                registry.addComponent<DamageOnCollision>(entity, {10});
                registry.addComponent<ScoreValueComponent>(entity, {100});

                ShooterComponent shooter;
                shooter.type = ShooterComponent::NORMAL;
                shooter.is_shooting = true;
                shooter.fire_rate = fire_rate;
                shooter.last_shot = 0.0f;
                shooter.projectile_damage = 15;

                if (shoot_pattern == "AIM_PLAYER") {
                    shooter.pattern = ShooterComponent::AIM_PLAYER;
                } else if (shoot_pattern == "SPREAD") {
                    shooter.pattern = ShooterComponent::SPREAD;
                } else if (shoot_pattern == "CIRCULAR") {
                    shooter.pattern = ShooterComponent::CIRCULAR;
                } else {
                    shooter.pattern = ShooterComponent::STRAIGHT;
                }
                registry.addComponent<ShooterComponent>(entity, shooter);

                if (shoot_pattern == "AIM_PLAYER") {
                    AIBehaviorComponent behavior;
                    behavior.shoot_at_player = true;
                    behavior.follow_player = false;
                    registry.addComponent<AIBehaviorComponent>(entity, behavior);
                }

                handle_t<TextureAsset> handle = texture_manager.load(sprite_path, TextureAsset(sprite_path));

                sprite2D_component_s sprite;
                sprite.handle = handle;
                sprite.dimension = {0, 0, 32, 32};
                sprite.z_index = 2;
                sprite.is_animated = true;
                sprite.loop_animation = true;
                sprite.animation_speed = 0.1f;
                for (int i = 0; i < 4; i++) {
                    sprite.frames.push_back({static_cast<float>(i * 32), 0, 32, 32});
                }
                registry.addComponent<sprite2D_component_s>(entity, sprite);

                auto& transform = registry.getComponent<transform_component_s>(entity);
                transform.scale_x = 2.0f;
                transform.scale_y = 2.0f;

                BoxCollisionComponent collision;
                collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
                collision.tagCollision.push_back("PLAYER");
                registry.addComponent<BoxCollisionComponent>(entity, collision);

                TagComponent tags;
                tags.tags.push_back("AI");
                tags.tags.push_back("TURRET");
                registry.addComponent<TagComponent>(entity, tags);
            });
    }
};
