#include <string>
#include "GameManager.hpp"
#include "ECS.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "src/RType/Common/Systems/spawn.hpp"
#include "src/RType/Common/Systems/health.hpp"
#include "src/RType/Common/Components/status_display_components.hpp"
#include "Components/StandardComponents.hpp"

static Entity findPlayerEntity(Registry& registry) {
    auto& entities = registry.getEntities<TagComponent>();
    for (auto entity : entities) {
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                return entity;
            }
        }
    }
    return -1;
}

void GameManager::updateUI(Environment& env) {
    auto& ecs = env.getECS();

    if (env.isServer() || _gameOver || _victory) {
        return;
    }

    Entity player_id = (_player) ? _player->getId() : findPlayerEntity(ecs.registry);

    // Update Status Display
    if (player_id != -1 && _statusDisplayEntity != static_cast<Entity>(-1)) {
        if (ecs.registry.hasComponent<StatusDisplayComponent>(_statusDisplayEntity)) {
            ecs.registry.getComponent<StatusDisplayComponent>(_statusDisplayEntity).player_entity = player_id;
        }
    }

    // Update Timer
    if (_timerEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_timerEntity)) {
        auto& game_timers = ecs.registry.getEntities<GameTimerComponent>();
        if (!game_timers.empty()) {
            auto& timer = ecs.registry.getComponent<GameTimerComponent>(game_timers[0]);
            ecs.registry.getComponent<TextComponent>(_timerEntity).text = 
                "Time: " + std::to_string(static_cast<int>(timer.elapsed_time)) + "s";
        }
    }

    // Update Boss HP
    if (_bossHPEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_bossHPEntity)) {
        auto& boss_hp_text = ecs.registry.getComponent<TextComponent>(_bossHPEntity);
        bool found_boss = false;
        
        auto& entities = ecs.registry.getEntities<TagComponent>();
        for (auto entity : entities) {
            auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
            if (std::find(tags.tags.begin(), tags.tags.end(), "BOSS") != tags.tags.end()) {
                if (ecs.registry.hasComponent<HealthComponent>(entity)) {
                    auto& health = ecs.registry.getComponent<HealthComponent>(entity);
                    float percent = (static_cast<float>(health.current_hp) / health.max_hp) * 100.0f;
                    boss_hp_text.text = "BOSS: " + std::to_string(health.current_hp) + "/" + 
                                       std::to_string(health.max_hp) + " (" + std::to_string(static_cast<int>(percent)) + "%)";
                    
                    // Couleur dynamique
                    if (percent > 50) boss_hp_text.color = sf::Color::Red;
                    else if (percent > 25) boss_hp_text.color = sf::Color(255, 165, 0); // Orange
                    else boss_hp_text.color = sf::Color::Magenta;
                    
                    found_boss = true;
                }
                break;
            }
        }
        if (!found_boss) boss_hp_text.text = "";
    }
}

void GameManager::checkGameState(Environment& env) {
    auto& ecs = env.getECS();

    if (_gameOver || _victory) {
        return;
    }

    // In client mode, game state is managed by server
    // But we can check for local game over condition (player death) for UI purposes

    Entity player_id = -1;
    if (_player) {
        player_id = _player->getId();
    } else {
        player_id = findPlayerEntity(ecs.registry);
    }

    if (player_id != -1) {
        if (ecs.registry.hasComponent<HealthComponent>(player_id)) {
            auto& health = ecs.registry.getConstComponent<HealthComponent>(player_id);
            if (health.current_hp < 1) {
                _gameOver = true;
                displayGameOver(env, false);
                return;
            }
        }
    } else if (!env.isClient()) {
        // Only on server/standalone: if player is missing, it's game over
        // On client, we might just be waiting for spawn
        if (_player) { // If we expected a player (standalone)
             _gameOver = true;
             displayGameOver(env, false);
             return;
        }
    }

    // Vérification de la victoire (boss tué)
    auto& entities = ecs.registry.getEntities<TagComponent>();
    bool boss_exists = false;
    bool boss_spawned = false;

    // Vérifier si le boss a été spawné
    auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
    for (auto spawner : spawners) {
        if (ecs.registry.hasComponent<EnemySpawnComponent>(spawner)) {
            auto& spawn_comp = ecs.registry.getConstComponent<EnemySpawnComponent>(spawner);
            if (spawn_comp.boss_arrived) {
                boss_spawned = true;
                break;
            }
        }
    }

    if (boss_spawned) {
        for (auto entity : entities) {
            auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
            for (const auto& tag : tags.tags) {
                if (tag == "BOSS") {
                    boss_exists = true;
                    break;
                }
            }
            if (boss_exists)
                break;
        }

        if (!boss_exists) {
            _victory = true;
            displayGameOver(env, true);
        }
    }
}

void GameManager::displayGameOver(Environment& env, bool victory) {
    if (env.isServer()) return;

    auto& ecs = env.getECS();
    UIConfig ui;
    try {
        ui = ConfigLoader::loadUIConfig("src/RType/Common/content/config/ui.cfg");
    } catch (...) {}

    auto& gom = ui.elements["GameOverMain"];
    auto& gos = ui.elements["GameOverScore"];
    std::string font = gom.font.empty() ? "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf" : gom.font;

    // Main Message
    _gameStateEntity = ecs.registry.createEntity();
    TextComponent mainText;
    mainText.text = victory ? "VICTORY!" : "GAME OVER";
    mainText.fontPath = font;
    mainText.characterSize = static_cast<unsigned int>(gom.size == 0 ? 72 : gom.size);
    mainText.color = victory ? sf::Color::Green : sf::Color::Red;
    mainText.x = gom.x == 0 ? 700 : gom.x;
    mainText.y = gom.y == 0 ? 450 : gom.y;
    ecs.registry.addComponent<TextComponent>(_gameStateEntity, mainText);

    // Final Score
    Entity scoreMessage = ecs.registry.createEntity();
    TextComponent scoreText;
    scoreText.text = "Final Score: " + std::to_string(ScoreSystem::getScore(ecs.registry));
    scoreText.fontPath = font;
    scoreText.characterSize = static_cast<unsigned int>(gos.size == 0 ? 36 : gos.size);
    scoreText.color = sf::Color::Yellow;
    scoreText.x = gos.x == 0 ? mainText.x + 50 : gos.x;
    scoreText.y = gos.y == 0 ? mainText.y + 100 : gos.y;
    ecs.registry.addComponent<TextComponent>(scoreMessage, scoreText);
}
