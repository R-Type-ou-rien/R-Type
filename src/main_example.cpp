
#include <iostream>
#include <string>
#include <vector>
#include "Engine/Core/ClientGameEngine.hpp"
// main pour le scene manager

// Mock Component for testing if not existing
struct Velocity {
    float x, y;
};

struct Position {
    float x, y;
};

// Mock Parsing Function (JSON -> LevelConfig)
LevelConfig parseLevel(const std::string& dummyPath) {
    LevelConfig config;
    config.name = "Level 1";
    config.background_texture = "bg.png";
    config.music_track = "theme.ogg";

    EntityConfig p1;
    p1.type = "Player";
    p1.properties["x"] = 100.0f;
    p1.properties["y"] = 200.0f;

    EntityConfig p2;
    p2.type = "Enemy";
    p2.properties["x"] = 500.0f;
    p2.properties["y"] = 100.0f;

    config.entities.push_back(p1);
    config.entities.push_back(p2);

    return config;
}

int main() {
    ClientGameEngine engine("R-Type Test");

    // 1. Register Prefabs via Engine
    std::cout << "--> Initializing SceneManager via Engine..." << std::endl;
    SceneManager& sceneMgr = engine.getSceneManager();

    sceneMgr.registerPrefab("Player",
                            [](Registry& reg, Entity e, const std::unordered_map<std::string, std::any>& props) {
                                std::cout << "Player created at " << std::any_cast<double>(props.at("x")) << ", "
                                          << std::any_cast<double>(props.at("y")) << std::endl;
                            });

    sceneMgr.registerPrefab("Enemy",
                            [](Registry& reg, Entity e, const std::unordered_map<std::string, std::any>& props) {
                                std::cout << "Enemy created at " << std::any_cast<double>(props.at("x")) << ", "
                                          << std::any_cast<double>(props.at("y")) << std::endl;
                            });

    // 2. Parsing (Mocked)
    std::cout << "--> Parsing Level..." << std::endl;
    LevelConfig levelCfg = parseLevel("level1.json");

    // 3. Engine Load
    std::cout << "--> Loading Scene..." << std::endl;
    sceneMgr.loadScene(levelCfg);

    // 4. Engine Run (Simulated via init/run if implemented, or just loop here for test)
    // engine.run();
    return 0;
}
