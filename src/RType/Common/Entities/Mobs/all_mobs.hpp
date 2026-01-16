#pragma once

#include <memory>
#include <map>
#include <string>
#include "mob_spawner.hpp"

class ScoutSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "SCOUT"; }
};

class FighterSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "FIGHTER"; }
};

class TankSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "TANK"; }
};

class ShooterSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "SHOOTER"; }
};

class KamikazeSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "KAMIKAZE"; }
};

class BossSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "BOSS"; }
};

class ObstacleSpawner : public IMobSpawner {
   public:
    void spawn(Registry& registry, system_context context, float x, float y, const EntityConfig& config) override;
    std::string getTypeName() const override { return "OBSTACLE"; }
};

class MobSpawnerFactory {
   public:
    static std::map<std::string, std::unique_ptr<IMobSpawner>> createSpawners() {
        std::map<std::string, std::unique_ptr<IMobSpawner>> spawners;
        spawners["SCOUT"] = std::make_unique<ScoutSpawner>();
        spawners["FIGHTER"] = std::make_unique<FighterSpawner>();
        spawners["TANK"] = std::make_unique<TankSpawner>();
        spawners["SHOOTER"] = std::make_unique<ShooterSpawner>();
        spawners["KAMIKAZE"] = std::make_unique<KamikazeSpawner>();
        spawners["BOSS"] = std::make_unique<BossSpawner>();
        spawners["OBSTACLE"] = std::make_unique<ObstacleSpawner>();
        return spawners;
    }
};
