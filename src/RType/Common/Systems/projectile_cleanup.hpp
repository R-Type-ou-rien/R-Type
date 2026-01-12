#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

// Système pour nettoyer les projectiles hors écran (optimisation performance)
class ProjectileCleanupSystem : public ISystem {
   public:
    ProjectileCleanupSystem() = default;
    ~ProjectileCleanupSystem() = default;
    void update(Registry& registry, system_context context) override;
};
