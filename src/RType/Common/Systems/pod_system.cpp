#include "pod_system.hpp"

#include <cmath>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "../Components/pod_component.hpp"
#include "Components/StandardComponents.hpp"
#include "ResourceConfig.hpp"
#include "damage.hpp"
#include "shooter.hpp"
#include "health.hpp"
#include "../Components/charged_shot.hpp"
#include "./animation_helper.hpp"

bool PodSystem::allPlayersHavePods(Registry& registry) {
    auto& players = registry.getEntities<TagComponent>();
    int player_count = 0;
    int players_with_pods = 0;

    for (auto entity : players) {
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        bool is_player = false;
        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                is_player = true;
                break;
            }
        }
        if (is_player) {
            player_count++;
            if (registry.hasComponent<PlayerPodComponent>(entity)) {
                auto& pod_comp = registry.getConstComponent<PlayerPodComponent>(entity);
                if (pod_comp.has_pod) {
                    players_with_pods++;
                }
            }
        }
    }

    return (player_count > 0 && player_count == players_with_pods);
}

void PodSystem::spawnPod(Registry& registry, system_context context) {
    Entity pod_id = registry.createEntity();
    const float world_w = static_cast<float>(context.window.getSize().x);
    const float world_h = static_cast<float>(context.window.getSize().y);
    float spawn_x = world_w + 50.0f;
    float spawn_y = 100.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * ((world_h - 200.0f));

    registry.addComponent<transform_component_s>(pod_id, {spawn_x, spawn_y, 1.0f, 1.0f});
    registry.addComponent<Velocity2D>(pod_id, {-80.0f, 0.0f});

    TagComponent tags;
    tags.tags.push_back("POD");
    tags.tags.push_back("ITEM");
    registry.addComponent<TagComponent>(pod_id, tags);

    PodComponent pod_comp;
    pod_comp.state = PodState::FLOATING;
    pod_comp.owner_id = static_cast<Entity>(-1);
    pod_comp.base_y = spawn_y;
    pod_comp.float_time = 0.0f;
    pod_comp.wave_amplitude = 30.0f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 40.0f;
    pod_comp.wave_frequency = 1.5f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 1.5f;
    registry.addComponent<PodComponent>(pod_id, pod_comp);
    registry.addComponent<TeamComponent>(pod_id, {TeamComponent::ALLY});

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(pod_id, collision);

    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet3.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet3.gif"));

    // Le sprite r-typesheet3.gif fait 205x18 pixels avec 6 frames
    // 205 / 6 = 34.16, donc chaque frame fait 34x18 (avec 1px de padding entre frames)
    constexpr float POD_FRAME_WIDTH = 34.0f;
    constexpr float POD_FRAME_HEIGHT = 18.0f;
    constexpr int POD_NUM_FRAMES = 6;
    constexpr float POD_SCALE = 3.0f;

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.z_index = 2;
    // Définir dimension pour le système de collision
    sprite_info.dimension = {0.0f, 0.0f, POD_FRAME_WIDTH, POD_FRAME_HEIGHT};

    registry.addComponent<sprite2D_component_s>(pod_id, sprite_info);

    // Animation: 6 frames horizontales, start à (1,0) avec padding de 0
    AnimationHelper::setupAnimation(registry, pod_id, 1.0f, 0.0f, POD_FRAME_WIDTH, POD_FRAME_HEIGHT, POD_NUM_FRAMES,
                                    0.12f, 0.0f);

    auto& transform = registry.getComponent<transform_component_s>(pod_id);
    transform.scale_x = POD_SCALE;
    transform.scale_y = POD_SCALE;

    std::cout << "[PodSystem] Pod spawned at (" << spawn_x << ", " << spawn_y << ")" << std::endl;
}

void PodSystem::updateFloatingPodMovement(Registry& registry, system_context context) {
    auto& pods = registry.getEntities<PodComponent>();

    for (auto pod_entity : pods) {
        auto& pod = registry.getComponent<PodComponent>(pod_entity);

        if (pod.state != PodState::FLOATING)
            continue;

        if (!registry.hasComponent<transform_component_s>(pod_entity))
            continue;
        auto& pos = registry.getComponent<transform_component_s>(pod_entity);

        pod.float_time += context.dt;

        pos.y = pod.base_y + std::sin(pod.float_time * pod.wave_frequency) * pod.wave_amplitude;

        const float world_h = static_cast<float>(context.window.getSize().y);
        float min_y = 80.0f;
        float max_y = world_h - 80.0f;

        if (pos.y < min_y) {
            pos.y = min_y;
            pod.base_y = min_y;
            pod.float_time = 0.0f;
        } else if (pos.y > max_y) {
            pos.y = max_y;
            pod.base_y = max_y;
            pod.float_time = 0.0f;
        }
    }
}

void PodSystem::handlePodCollection(Registry& registry, system_context context) {
    auto& pods = registry.getEntities<PodComponent>();
    auto& players = registry.getEntities<TagComponent>();

    std::vector<Entity> pods_to_collect;

    for (auto pod_entity : pods) {
        auto& pod = registry.getComponent<PodComponent>(pod_entity);

        if (pod.state != PodState::FLOATING)
            continue;

        if (!registry.hasComponent<BoxCollisionComponent>(pod_entity))
            continue;
        auto& pod_collision = registry.getConstComponent<BoxCollisionComponent>(pod_entity);

        for (Entity collided_entity : pod_collision.collision.tags) {
            if (!registry.hasComponent<TagComponent>(collided_entity))
                continue;
            auto& collided_tags = registry.getConstComponent<TagComponent>(collided_entity);

            bool is_player = false;
            for (const auto& tag : collided_tags.tags) {
                if (tag == "PLAYER") {
                    is_player = true;
                    break;
                }
            }
            if (!is_player)
                continue;

            Entity player_entity = collided_entity;

            if (registry.hasComponent<PlayerPodComponent>(player_entity)) {
                auto& player_pod = registry.getComponent<PlayerPodComponent>(player_entity);
                if (player_pod.has_pod) {
                    continue;
                }
            }

            pod.state = PodState::ATTACHED;
            pod.owner_id = player_entity;

            if (!registry.hasComponent<PlayerPodComponent>(player_entity)) {
                PlayerPodComponent new_pod_comp;
                new_pod_comp.has_pod = true;
                new_pod_comp.pod_entity = pod_entity;
                new_pod_comp.pod_attached = true;
                registry.addComponent<PlayerPodComponent>(player_entity, new_pod_comp);
            } else {
                auto& player_pod = registry.getComponent<PlayerPodComponent>(player_entity);
                player_pod.has_pod = true;
                player_pod.pod_entity = pod_entity;
                player_pod.pod_attached = true;
            }

            auto& pod_col = registry.getComponent<BoxCollisionComponent>(pod_entity);
            pod_col.tagCollision.clear();
            pod_col.tagCollision.push_back("AI");

            registry.addComponent<DamageOnCollision>(pod_entity, {50});

            if (registry.hasComponent<ChargedShotComponent>(player_entity)) {
                registry.removeComponent<ChargedShotComponent>(player_entity);
            }

            if (registry.hasComponent<ShooterComponent>(player_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(player_entity);
                shooter.use_pod_laser = true;
            }

            if (registry.hasComponent<Velocity2D>(pod_entity)) {
                auto& vel = registry.getComponent<Velocity2D>(pod_entity);
                vel.vx = 0;
                vel.vy = 0;
            }

            std::cout << "[PodSystem] Player " << player_entity << " collected pod " << pod_entity << std::endl;
            break;
        }
    }
}

void PodSystem::updateAttachedPodPosition(Registry& registry, system_context context) {
    auto& pods = registry.getEntities<PodComponent>();

    for (auto pod_entity : pods) {
        auto& pod = registry.getComponent<PodComponent>(pod_entity);

        if (pod.state != PodState::ATTACHED)
            continue;
        if (pod.owner_id == -1)
            continue;

        if (!registry.hasComponent<transform_component_s>(pod.owner_id))
            continue;
        if (!registry.hasComponent<transform_component_s>(pod_entity))
            continue;

        auto& player_pos = registry.getConstComponent<transform_component_s>(pod.owner_id);
        auto& pod_pos = registry.getComponent<transform_component_s>(pod_entity);

        pod_pos.x = player_pos.x + 60.0f;
        pod_pos.y = player_pos.y;
    }
}

void PodSystem::updateDetachedPodPosition(Registry& registry, system_context context) {
    auto& pods = registry.getEntities<PodComponent>();

    for (auto pod_entity : pods) {
        auto& pod = registry.getComponent<PodComponent>(pod_entity);

        if (pod.state != PodState::DETACHED)
            continue;
        if (pod.owner_id == -1)
            continue;

        if (!registry.hasComponent<transform_component_s>(pod.owner_id))
            continue;
        if (!registry.hasComponent<transform_component_s>(pod_entity))
            continue;

        auto& player_pos = registry.getConstComponent<transform_component_s>(pod.owner_id);
        auto& pod_pos = registry.getComponent<transform_component_s>(pod_entity);

        float target_x = player_pos.x + 80.0f;
        float target_y = player_pos.y;

        float follow_speed = 200.0f * context.dt;
        float dx = target_x - pod_pos.x;
        float dy = target_y - pod_pos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 5.0f) {
            pod_pos.x += (dx / distance) * follow_speed;
            pod_pos.y += (dy / distance) * follow_speed;
        }
    }
}

void PodSystem::handlePlayerDamage(Registry& registry, system_context context) {
    auto& players = registry.getEntities<PlayerPodComponent>();

    for (auto player_entity : players) {
        auto& player_pod = registry.getComponent<PlayerPodComponent>(player_entity);

        if (!registry.hasComponent<HealthComponent>(player_entity))
            continue;
        auto& health = registry.getConstComponent<HealthComponent>(player_entity);

        if (player_pod.last_known_hp == -1) {
            player_pod.last_known_hp = health.current_hp;
        }

        if (!player_pod.has_pod || !player_pod.pod_attached) {
            player_pod.last_known_hp = health.current_hp;
            continue;
        }

        if (health.current_hp < player_pod.last_known_hp) {
            Entity pod_entity = player_pod.pod_entity;

            if (registry.hasComponent<HealthComponent>(player_entity)) {
                auto& player_health = registry.getComponent<HealthComponent>(player_entity);
                player_health.current_hp = player_pod.last_known_hp;  // Restaurer les HP
            }

            if (registry.hasComponent<ShooterComponent>(player_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(player_entity);
                shooter.use_pod_laser = false;
            }

            if (!registry.hasComponent<ChargedShotComponent>(player_entity)) {
                ChargedShotComponent charged_shot;
                charged_shot.medium_charge_threshold = 1.0f;  // 50% = yellow bar
                charged_shot.max_charge_time = 2.0f;          // 100% = full red bar
                registry.addComponent<ChargedShotComponent>(player_entity, charged_shot);
            }

            if (registry.hasComponent<PodComponent>(pod_entity)) {
                registry.destroyEntity(pod_entity);
            }

            player_pod.has_pod = false;
            player_pod.pod_entity = -1;
            player_pod.pod_attached = false;
            player_pod.last_known_hp = health.current_hp;

            std::cout << "[PodSystem] Pod destroyed! Player " << player_entity << " was hit with pod attached."
                      << std::endl;
        } else {
            player_pod.last_known_hp = health.current_hp;
        }
    }
}

void PodSystem::handlePodToggle(Registry& registry, system_context context) {
    auto& players = registry.getEntities<PlayerPodComponent>();

    for (auto player_entity : players) {
        auto& player_pod = registry.getComponent<PlayerPodComponent>(player_entity);

        if (!player_pod.has_pod || !player_pod.detach_requested)
            continue;
        player_pod.detach_requested = false;

        Entity pod_entity = player_pod.pod_entity;
        if (!registry.hasComponent<PodComponent>(pod_entity))
            continue;

        auto& pod = registry.getComponent<PodComponent>(pod_entity);

        if (player_pod.pod_attached) {
            pod.state = PodState::DETACHED;
            player_pod.pod_attached = false;

            if (registry.hasComponent<ShooterComponent>(player_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(player_entity);
                shooter.use_pod_laser = false;
            }

            if (!registry.hasComponent<ChargedShotComponent>(player_entity)) {
                ChargedShotComponent charged_shot;
                charged_shot.medium_charge_threshold = 1.0f;  // 50% = yellow bar
                charged_shot.max_charge_time = 2.0f;          // 100% = full red bar
                registry.addComponent<ChargedShotComponent>(player_entity, charged_shot);
            }

            if (registry.hasComponent<DamageOnCollision>(pod_entity)) {
                registry.removeComponent<DamageOnCollision>(pod_entity);
            }

            if (registry.hasComponent<BoxCollisionComponent>(pod_entity)) {
                auto& collision = registry.getComponent<BoxCollisionComponent>(pod_entity);
                collision.tagCollision.clear();
            }

            std::cout << "[PodSystem] Pod " << pod_entity << " detached from player " << player_entity << std::endl;
        } else {
            pod.state = PodState::ATTACHED;
            player_pod.pod_attached = true;

            if (registry.hasComponent<ShooterComponent>(player_entity)) {
                auto& shooter = registry.getComponent<ShooterComponent>(player_entity);
                shooter.use_pod_laser = true;
            }

            if (registry.hasComponent<ChargedShotComponent>(player_entity)) {
                registry.removeComponent<ChargedShotComponent>(player_entity);
            }

            if (!registry.hasComponent<DamageOnCollision>(pod_entity)) {
                registry.addComponent<DamageOnCollision>(pod_entity, {50});
            }

            if (registry.hasComponent<BoxCollisionComponent>(pod_entity)) {
                auto& collision = registry.getComponent<BoxCollisionComponent>(pod_entity);
                collision.tagCollision.clear();
                collision.tagCollision.push_back("AI");
            }

            std::cout << "[PodSystem] Pod " << pod_entity << " attached to player " << player_entity << std::endl;
        }
    }
}

void PodSystem::createPodLaserProjectile(Registry& registry, system_context context, transform_component_s pos,
                                         float angle, int damage) {
    Entity projectile_id = registry.createEntity();

    float speed = 600.0f;
    float vx = std::cos(angle) * speed;
    float vy = std::sin(angle) * speed;

    registry.addComponent<transform_component_s>(projectile_id, {pos.x, pos.y, 1.5f, 1.5f});
    registry.addComponent<Velocity2D>(projectile_id, {vx, vy});

    TagComponent tags;
    tags.tags.push_back("FRIENDLY_PROJECTILE");
    tags.tags.push_back("POD_LASER");
    registry.addComponent<TagComponent>(projectile_id, tags);

    registry.addComponent<TeamComponent>(projectile_id, {TeamComponent::ALLY});
    registry.addComponent<ProjectileComponent>(projectile_id, {static_cast<int>(projectile_id)});
    registry.addComponent<DamageOnCollision>(projectile_id, {damage});

    // a faire remplacer avec le sprite du laser circulaire du pod
    handle_t<TextureAsset> handle =
        context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
                                     TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.animation_speed = 0;
    sprite_info.current_animation_frame = 0;
    // a faire mettre les coordonées correctes du sprite laser circulaire du pod
    sprite_info.dimension = {232, 103, 32, 14};
    sprite_info.z_index = 3;

    registry.addComponent<sprite2D_component_s>(projectile_id, sprite_info);

    auto& transform = registry.getComponent<transform_component_s>(projectile_id);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("AI");
    registry.addComponent<BoxCollisionComponent>(projectile_id, collision);

    AudioSourceComponent audio;
    audio.sound_name = "pod_laser";  // a faire ajouter le son du tir de laser du pod
    audio.play_on_start = true;
    audio.loop = false;
    audio.destroy_entity_on_finish = false;
    registry.addComponent<AudioSourceComponent>(projectile_id, audio);
}

void PodSystem::handleDetachedPodShooting(Registry& registry, system_context context) {
    auto& pods = registry.getEntities<PodComponent>();

    for (auto pod_entity : pods) {
        auto& pod = registry.getComponent<PodComponent>(pod_entity);

        if (pod.state != PodState::DETACHED)
            continue;

        pod.last_shot_time += context.dt;

        if (pod.last_shot_time >= pod.auto_fire_rate) {
            pod.last_shot_time = 0.0f;

            if (!registry.hasComponent<transform_component_s>(pod_entity))
                continue;
            auto& pod_pos = registry.getConstComponent<transform_component_s>(pod_entity);

            Entity projectile_id = registry.createEntity();

            registry.addComponent<transform_component_s>(projectile_id, {pod_pos.x + 30.0f, pod_pos.y, 1.0f, 1.0f});
            registry.addComponent<Velocity2D>(projectile_id, {700.0f, 0.0f});

            TagComponent tags;
            tags.tags.push_back("FRIENDLY_PROJECTILE");
            registry.addComponent<TagComponent>(projectile_id, tags);

            registry.addComponent<TeamComponent>(projectile_id, {TeamComponent::ALLY});
            registry.addComponent<ProjectileComponent>(projectile_id, {static_cast<int>(projectile_id)});
            registry.addComponent<DamageOnCollision>(projectile_id, {pod.projectile_damage});

            handle_t<TextureAsset> handle =
                context.texture_manager.load("src/RType/Common/content/sprites/r-typesheet1.gif",
                                             TextureAsset("src/RType/Common/content/sprites/r-typesheet1.gif"));

            sprite2D_component_s sprite_info;
            sprite_info.handle = handle;
            sprite_info.animation_speed = 0;
            sprite_info.current_animation_frame = 0;
            sprite_info.dimension = {232, 103, 32, 14};
            sprite_info.z_index = 1;

            registry.addComponent<sprite2D_component_s>(projectile_id, sprite_info);

            BoxCollisionComponent collision;
            collision.tagCollision.push_back("AI");
            registry.addComponent<BoxCollisionComponent>(projectile_id, collision);

            AudioSourceComponent audio;
            audio.sound_name = "shoot";
            audio.play_on_start = true;
            audio.loop = false;
            audio.destroy_entity_on_finish = false;
            registry.addComponent<AudioSourceComponent>(projectile_id, audio);
        }
    }
}

void PodSystem::update(Registry& registry, system_context context) {
    auto& spawners = registry.getEntities<PodSpawnComponent>();
    if (spawners.empty()) {
        static bool warned = false;
        if (!warned) {
            std::cout << "[PodSystem] WARNING: No PodSpawnComponent found! Pod spawning disabled." << std::endl;
            warned = true;
        }
        return;
    }
    for (auto spawner : spawners) {
        auto& spawn_comp = registry.getComponent<PodSpawnComponent>(spawner);

        if (allPlayersHavePods(registry)) {
            spawn_comp.can_spawn = false;
            continue;
        } else {
            spawn_comp.can_spawn = true;
        }

        bool floating_pod_exists = false;
        auto& pods = registry.getEntities<PodComponent>();
        for (auto pod : pods) {
            auto& pod_comp = registry.getConstComponent<PodComponent>(pod);
            if (pod_comp.state == PodState::FLOATING) {
                floating_pod_exists = true;
                break;
            }
        }

        if (floating_pod_exists)
            continue;

        spawn_comp.spawn_timer += context.dt;
        if (spawn_comp.spawn_timer >= spawn_comp.spawn_interval && spawn_comp.can_spawn) {
            spawn_comp.spawn_timer = 0.0f;
            spawn_comp.spawn_interval =
                spawn_comp.min_spawn_interval + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) *
                                                    (spawn_comp.max_spawn_interval - spawn_comp.min_spawn_interval);
            spawnPod(registry, context);
        }
    }

    updateFloatingPodMovement(registry, context);
    handlePodCollection(registry, context);
    handlePlayerDamage(registry, context);
    handlePodToggle(registry, context);
    updateAttachedPodPosition(registry, context);
    updateDetachedPodPosition(registry, context);
    handleDetachedPodShooting(registry, context);
    auto& pods = registry.getEntities<PodComponent>();
    std::vector<Entity> to_destroy;
    for (auto pod : pods) {
        auto& pod_comp = registry.getConstComponent<PodComponent>(pod);
        if (pod_comp.state == PodState::FLOATING) {
            if (registry.hasComponent<transform_component_s>(pod)) {
                auto& pos = registry.getConstComponent<transform_component_s>(pod);
                if (pos.x < -100.0f) {
                    to_destroy.push_back(pod);
                }
            }
        }
    }
    for (auto pod : to_destroy) {
        registry.destroyEntity(pod);
    }
}
