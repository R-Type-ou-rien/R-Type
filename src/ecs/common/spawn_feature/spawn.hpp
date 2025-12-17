#pragma once

#include <functional>
#include <unordered_map>

#include "../Components/Components.hpp"
#include "../../Engine/Core/ECS/ISystem.hpp"
#include "../../Engine/Core/ECS/Registry/registry.hpp"

struct SpawnComponent {
    double time_until_spawn;
    bool active = true;
    TypeEntityComponent type;
    transform_component_s transform;
    Velocity2D initial_velocity;
};

class SpawnSystem : public ISystem {
   public:
    SpawnSystem();
    ~SpawnSystem() = default;

    void init(Registry& registry) {}
    void update(Registry& registry, system_context context) override;

   private:
    void create_entity(Registry& registry, const SpawnComponent& info);

    using EntityFactory = std::function<void(Registry&, Entity)>;
    std::unordered_map<TypeEntityComponent::TypeEntity, EntityFactory> _factories;
};
