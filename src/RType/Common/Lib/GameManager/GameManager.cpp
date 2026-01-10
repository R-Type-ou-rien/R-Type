#include "GameManager.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include "ClientGameEngine.hpp"
#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "GameEngineConfig.hpp"
#include "InputConfig.hpp"
#include "Network.hpp"
#include "ResourceConfig.hpp"
#include "src/RType/Common/Components/charged_shot.hpp"
#include "src/RType/Common/Components/damage.hpp"
#include "src/RType/Common/Components/health.hpp"
#include "src/RType/Common/Components/shooter.hpp"
#include "src/RType/Common/Components/spawn.hpp"
#include "src/RType/Common/Lib/Actors/Player/Player.hpp"
#include "src/RType/Common/Lib/Actors/AI/AI.hpp"

GameManager::GameManager(uint32_t requiredPlayers) : _requiredPlayers(requiredPlayers) {}

bool GameManager::isGameReady(GameEngine& engine) {
    if (_gameStarted) return true;

    auto lobby = engine.getLobbyManager().getLobby(1); // Assuming lobby 1 is the game lobby
    if (lobby && lobby->get().getPlayerCount() >= _requiredPlayers) {
        if (lobby->get().getState() == engine::core::Lobby::State::WAITING) {
            std::cout << "GAME MANAGER: Starting game!" << std::endl;
            lobby->get().setState(engine::core::Lobby::State::IN_GAME);
            
            // This is where we should create the players for the clients
            initPlayers(engine);
            initEnemies(engine);
            initSpawner(engine);

            _gameStarted = true;
        }
        return true;
    }
    return false;
}

void GameManager::initSystems(GameEngine& engine) {
    auto& ecs = engine.getECS();
    ecs.systems.addSystem<ShooterSystem>();
    ecs.systems.addSystem<Damage>();
    ecs.systems.addSystem<HealthSystem>();
    ecs.systems.addSystem<PatternSystem>();
    ecs.systems.addSystem<EnemySpawnSystem>();
}

void GameManager::initBackground(GameEngine& engine) {
    if (engine.IsServer) return;

    const std::string bgPath = "content/sprites/background-R-Type.png";
    auto& ecs = engine.getECS();
    Entity bgEntity = ecs.registry.createEntity();

    BackgroundComponent bg{};
    bg.x_offset = 0.f;
    bg.scroll_speed = 60.f;

    // The texture manager is now part of the engine
    bg.texture_handle = engine.getTextureManager().load(bgPath, TextureAsset(bgPath));

    ecs.registry.addComponent(bgEntity, bg);
}

void GameManager::initPlayers(GameEngine& engine) {
    auto& ecs = engine.getECS();
    auto& lobby = engine.getLobbyManager().getLobby(1).value().get();
    auto& clientToEntityMap = engine.getClientToEntityMap();

    float startY = PLAYER_START_Y;
    for (const auto& client : lobby.getClients()) {
        // Create a new Player object
        auto newPlayer = std::make_unique<Player>(ecs, engine.getTextureManager(), std::pair<float, float>{PLAYER_START_X, startY});
        
        // Configure the player
        newPlayer->setTexture("content/sprites/r-typesheet42.gif");
        newPlayer->setTextureDimension(rect{0, 0, 32, 16});
        newPlayer->setFireRate(0.5);
        newPlayer->setLifePoint(PLAYER_MAX_HP);
        
        newPlayer->addCollisionTag("AI");
        newPlayer->addCollisionTag("ENEMY_PROJECTILE");
        
        // Add a charged shot component
        ChargedShotComponent charged_shot;
        charged_shot.min_charge_time = 0.5f;
        charged_shot.max_charge_time = 2.0f;
        ecs.registry.addComponent<ChargedShotComponent>(newPlayer->getId(), charged_shot);

        NetworkIdentity& comp = ecs.registry.getComponent<NetworkIdentity>(newPlayer->getId());
        comp.ownerId = client.id;

        // Map the client ID to the entity ID for input handling
        clientToEntityMap[client.id] = newPlayer->getId();
        std::cout << "GAME MANAGER: Created player entity " << newPlayer->getId() << " for client " << client.id << std::endl;

        // Notify the client which entity is theirs
        network::message<network::GameEvents> msg;
        msg.header.id = network::GameEvents::S_ASSIGN_PLAYER_ENTITY;
        network::AssignPlayerEntityPacket packet;
        packet.entityId = newPlayer->getId();
        msg << packet;
        engine.getNetwork().transmitEvent(network::GameEvents::S_ASSIGN_PLAYER_ENTITY, msg, client.id);

        startY += 50.0f; // Stagger players
    }
}

void GameManager::initEnemies(GameEngine& engine) {
    if (!engine.IsServer) return;
    
    // The direct instantiation replaces env.spawn()
    auto enemy = std::make_unique<AI>(engine.getECS(), engine.getTextureManager(), std::pair<float, float>(600.f, 200.f));
    enemy->setTextureEnemy("content/sprites/r-typesheet8.gif");
    enemy->setPatternType(PatternComponent::SINUSOIDAL);
    enemy->setLifePoint(10);
    enemy->setCurrentHealth(10);
    enemy->addCollisionTag("FRIENDLY_PROJECTILE");
    enemy->addCollisionTag("PLAYER");
}

void GameManager::initSpawner(GameEngine& engine) {
    if (!engine.IsServer) return;

    auto& ecs = engine.getECS();
    Entity spawner = ecs.registry.createEntity();
    EnemySpawnComponent spawn_comp;
    spawn_comp.spawn_interval = 5.0f;
    spawn_comp.enemies_per_wave = 3;
    spawn_comp.is_active = true;
    ecs.registry.addComponent(spawner, spawn_comp);
}

void GameManager::initUI(GameEngine& engine) {
    if (engine.IsServer) return;

    auto& ecs = engine.getECS();
    _uiEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _uiEntity, {"Waiting for players...", "content/open_dyslexic/OpenDyslexic-Regular.otf", 30, sf::Color::White, 10, 10});
}

void GameManager::init(GameEngine& engine, InputManager& inputs) {
    initSystems(engine);
    initBackground(engine);
    initUI(engine);
}

void GameManager::loadInputSetting(InputManager& inputs) {
    inputs.bindAction("move_left", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Left});
    inputs.bindAction("move_right", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Right});
    inputs.bindAction("move_up", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Up});
    inputs.bindAction("move_down", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Down});
    inputs.bindAction("shoot", InputBinding{InputDeviceType::Keyboard, sf::Keyboard::Key::Space});
}

void GameManager::updateUI(GameEngine& engine) {
    if (engine.IsServer) return;

    auto& ecs = engine.getECS();

    if (!ecs.registry.hasComponent<TextComponent>(_uiEntity)) {
        return;
    }
    auto& text = ecs.registry.getComponent<TextComponent>(_uiEntity);

    if (!_gameStarted) {
        text.text = "Waiting for players...";
        return;
    }
    
    auto localPlayerEntity = engine.getLocalPlayerEntity();
    if (localPlayerEntity.has_value()) {
        if (ecs.registry.hasComponent<HealthComponent>(localPlayerEntity.value())) {
            auto& health = ecs.registry.getComponent<HealthComponent>(localPlayerEntity.value());
            text.text = "HP: " + std::to_string(health.current_hp);
        } else {
             text.text = "HP: N/A";
        }
    } else {
         text.text = "GAME OVER";
    }
}

void GameManager::update(GameEngine& engine, InputManager& inputs) {
    if constexpr (GameEngine::IsServer) {
        // This is server-side logic
        auto& serverEngine = static_cast<ServerGameEngine&>(engine);
        if (isGameReady(serverEngine)) {
            // Game is running, update game state
        } else {
            // Game is waiting to start
        }
    } else {
        updateUI(engine);
    }
}
