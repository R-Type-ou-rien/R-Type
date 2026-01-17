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
#include "Components/NetworkComponents.hpp"
#include "../Components/terrain_component.hpp"
#include "../Components/team_component.hpp"
#include "../Systems/health.hpp"
#include "../Systems/shooter.hpp"
#include "../Systems/behavior.hpp"
#include "../Systems/score.hpp"

class ScenePrefabs {
   public:
    static void registerAll(SceneManager& scene_manager, ResourceManager<TextureAsset>& texture_manager) {
        registerWallPrefab(scene_manager, texture_manager);
        registerTurretPrefab(scene_manager, texture_manager);
        registerDecorPrefab(scene_manager, texture_manager);
    }

   private:
    static void registerWallPrefab(SceneManager& scene_manager, ResourceManager<TextureAsset>& texture_manager) {
        scene_manager.registerPrefab(
            "Wall", [&texture_manager](Registry& registry, Entity entity,
                                       const std::unordered_map<std::string, std::any>& props) {
                float x = 0, y = 0, width = 64, height = 64;
                std::string sprite_path = "src/RType/Common/content/sprites/wall-level1.gif";
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
                
                float scroll_speed = -100.0f;
                if (props.count("scroll_speed"))
                    scroll_speed = std::any_cast<float>(props.at("scroll_speed"));
                registry.addComponent<Velocity2D>(entity, {scroll_speed, 0.0f});

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

                // sprite2D_component_s sprite;
                // sprite.handle = handle;
                // sprite.dimension = {0, 0, 32, 32};
                // sprite.z_index = 3;
                // registry.addComponent<sprite2D_component_s>(entity, sprite);

                AnimatedSprite2D animation;
                AnimationClip clip;

                clip.handle = handle;
                clip.frames.emplace_back(0, 0, 32, 32);
                animation.layer = RenderLayer::Foreground;
                animation.animations.emplace("idle", clip);
                animation.currentAnimation = "idle";
                registry.addComponent<AnimatedSprite2D>(entity, animation);

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

                // Add NetworkIdentity for network replication
                registry.addComponent<NetworkIdentity>(entity, {static_cast<uint32_t>(entity), 0});
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
                
                float scroll_speed = 0.0f;
                if (props.count("scroll_speed"))
                    scroll_speed = std::any_cast<float>(props.at("scroll_speed"));
                registry.addComponent<Velocity2D>(entity, {scroll_speed, 0.0f});

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
                shooter.projectile_scale = 3.0f;

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
                    BehaviorComponent behavior;
                    behavior.shoot_at_player = true;
                    behavior.follow_player = false;
                    behavior.follow_speed = 100.0f;
                    registry.addComponent<BehaviorComponent>(entity, behavior);
                }

                handle_t<TextureAsset> handle = texture_manager.load(sprite_path, TextureAsset(sprite_path));

                // sprite2D_component_s sprite;
                // sprite.handle = handle;
                // sprite.dimension = {0, 0, 32, 32};
                // sprite.z_index = 2;
                // sprite.is_animated = true;
                // sprite.loop_animation = true;
                // sprite.animation_speed = 0.1f;
                // for (int i = 0; i < 4; i++) {
                //     sprite.frames.push_back({static_cast<float>(i * 32), 0, 32, 32});
                // }
                // registry.addComponent<sprite2D_component_s>(entity, sprite);

                AnimatedSprite2D animation;
                AnimationClip clip;

                clip.handle = handle;
                animation.layer = RenderLayer::Midground;
                clip.frameDuration = 0.1f;
                for (int i = 0; i < 4; i++) {
                    clip.frames.emplace_back(static_cast<float>(i * 32), 0, 32, 32);
                }
                animation.animations.emplace("idle", clip);
                animation.currentAnimation = "idle";
                registry.addComponent<AnimatedSprite2D>(entity, animation);

                auto& transform = registry.getComponent<transform_component_s>(entity);
                transform.scale_x = 2.5f;
                transform.scale_y = 2.5f;

                BoxCollisionComponent collision;
                collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
                collision.tagCollision.push_back("PLAYER");
                registry.addComponent<BoxCollisionComponent>(entity, collision);

                TagComponent tags;
                tags.tags.push_back("AI");
                tags.tags.push_back("TURRET");
                registry.addComponent<TagComponent>(entity, tags);

                registry.addComponent<NetworkIdentity>(entity, {static_cast<uint32_t>(entity), 0});
            });
    }

    static void registerDecorPrefab(SceneManager& scene_manager, ResourceManager<TextureAsset>& texture_manager) {
        scene_manager.registerPrefab(
            "Decor", [&texture_manager](Registry& registry, Entity entity,
                                        const std::unordered_map<std::string, std::any>& props) {
                float x = 0, y = 0;
                std::string sprite_path;
                float scale = 1.0f;
                int z_index = 0;
                float scroll_speed_mult = 1.0f;

                if (props.count("x"))
                    x = std::any_cast<float>(props.at("x"));
                if (props.count("y"))
                    y = std::any_cast<float>(props.at("y"));
                if (props.count("sprite"))
                    sprite_path = std::any_cast<std::string>(props.at("sprite"));
                if (props.count("scale"))
                    scale = std::any_cast<float>(props.at("scale"));
                if (props.count("z_index"))
                    z_index = std::any_cast<int>(props.at("z_index"));
                if (props.count("scroll_speed_mult"))
                    scroll_speed_mult = std::any_cast<float>(props.at("scroll_speed_mult"));

                registry.addComponent<transform_component_s>(entity, {x, y, scale, scale});       
                registry.addComponent<Velocity2D>(entity, {-100.0f * scroll_speed_mult, 0.0f});

                if (!sprite_path.empty()) {
                    handle_t<TextureAsset> handle = texture_manager.load(sprite_path, TextureAsset(sprite_path));
                    // sprite2D_component_s sprite;
                    // sprite.handle = handle;
                    // sprite.z_index = z_index;
                    // sprite.dimension = {0, 0, 0, 0};
                    // registry.addComponent<sprite2D_component_s>(entity, sprite);
                    AnimatedSprite2D animation;
                    AnimationClip clip;
                    
                    clip.handle = handle;
                    animation.layer = RenderLayer::Midground;
                    clip.frames.emplace_back(0, 0, 0, 0);
                    animation.currentAnimation = "idle";
                    animation.animations.emplace("idle", clip);
                    registry.addComponent<AnimatedSprite2D>(entity, animation);
                }
                registry.addComponent<NetworkIdentity>(entity, {static_cast<uint32_t>(entity), 0});
            });
    }
};
