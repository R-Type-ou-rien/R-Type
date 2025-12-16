#include <memory>
#include <vector>
#include <SFML/System/Clock.hpp>
#include "../Actors/Player/Player.hpp"
#include "../Actors/AI/AI.hpp"
#include "ECS.hpp"


class GameManager {
    private:
        std::unique_ptr<Player> _player;
        std::vector<std::unique_ptr<AI>> _ennemies;
        // Gestion de l'événement Boss
        sf::Clock _bossClock;          // Chrono depuis le début
        bool _bossSpawned = false;     // Le boss a-t-il été spawné ?
        std::unique_ptr<AI> _boss;     // Référence vers l'entité Boss
        // Paramètres configurables du boss
        float _bossTriggerSeconds = 120.f; // délai par défaut (2 minutes)
        std::string _bossSpritePath = "content/sprites/r-typesheet30.gif";
        rect _bossFrame {0, 0, 32, 32};
        std::pair<float, float> _bossScale {4.f, 4.f};
        double _bossFireRate = 0.0; // statique (pas d’animation/tir pour l’instant)
    
    public:
        GameManager();
        void init(ECS& ecs);
        void update(ECS& ecs);
        void loadInputSetting(ECS& ecs);

        // Setters pour rendre l'événement flexible
        void setBossTriggerTime(float seconds) { _bossTriggerSeconds = seconds; }
        void setBossSpritePath(const std::string& path) { _bossSpritePath = path; }
        void setBossFrame(rect frame) { _bossFrame = frame; }
        void setBossScale(std::pair<float, float> scale) { _bossScale = scale; }
        void setBossFireRate(double rate) { _bossFireRate = rate; }

};