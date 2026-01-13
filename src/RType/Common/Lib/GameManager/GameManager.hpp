#pragma once

#include <memory>
#include <vector>
#include <string>
#include <SFML/System/Clock.hpp>
#include "../../Entities/Player/Player.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/RType/Common/Components/config.hpp"
#include "src/RType/Common/Components/game_timer.hpp"
#include "src/Engine/Core/Scene/SceneManager.hpp"

/**
 * @brief GameManager gère la logique globale de la partie R-Type.
 * 
 * Il orchestre l'initialisation des systèmes, le chargement des scènes,
 * la création du joueur et de l'UI, ainsi que la vérification de l'état du jeu.
 */
class GameManager {
   private:
    // Entités et Gestionnaires
    std::unique_ptr<Player> _player;
    std::unique_ptr<SceneManager> _scene_manager;
    Entity _timerEntity;
    Entity _bossHPEntity;  // Nouvelle entité pour la vie du boss
    Entity _gameStateEntity;
    Entity _boundsEntity;
    Entity _scoreTrackerEntity;
    Entity _statusDisplayEntity;
    Entity _chargeBarEntity;
    Entity _livesEntity;
    Entity _scoreDisplayEntity;
    Entity _leaderboardEntity;
    bool _gameOver = false;
    bool _victory = false;
    bool _leaderboardDisplayed = false;

    // Configuration
    EntityConfig _player_config;
    GameConfig _game_config;
    std::string _current_level_scene;

    // Méthodes d'initialisation internes
    void initSystems(Environment& env);
    void initBackground(Environment& env, const LevelConfig& config);
    void initPlayer(Environment& env);
    void initSpawner(Environment& env, const LevelConfig& config);
    void initScene(Environment& env, const LevelConfig& config);
    void initUI(Environment& env);
    void initBounds(Environment& env);

    // Configuration des contrôles
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void setupPodControls(InputManager& inputs);

    // Mise à jour de la logique de jeu
    void updateUI(Environment& env);
    void checkGameState(Environment& env);
    void displayGameOver(Environment& env, bool victory);
    void displayLeaderboard(Environment& env, bool victory);

   public:
    GameManager();
    ~GameManager() = default;

    /**
     * @brief Initialise le jeu (Systèmes, Ressources, Scène, Entités).
     */
    void init(Environment& env, InputManager& inputs);

    /**
     * @brief Met à jour l'état du jeu à chaque frame.
     */
    void update(Environment& env, InputManager& inputs);

    /**
     * @brief Charge les paramètres d'entrée pour le joueur.
     */
    void loadInputSetting(InputManager& inputs);
};
