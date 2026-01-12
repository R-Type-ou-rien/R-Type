#include <utility>
#include "Player.hpp"
#include "src/RType/Common/Systems/health.hpp"
#include "src/RType/Common/Systems/damage.hpp"
#include "CollisionSystem.hpp"

Player::Player(ECS& ecs, ResourceManager<TextureAsset>& textures, std::pair<float, float> pos)
    : DynamicActor(ecs, true, textures, "PLAYER") {
    ResourceStat lifepoint;
    lifepoint.max = 100;
    lifepoint.current = 100;
    lifepoint.regenRate = 0;

    addResourceStat("lifepoint", lifepoint);
    setPosition(pos);
    _ecs.registry.addComponent<TeamComponent>(_id, {TeamComponent::Team::ALLY});
    _ecs.registry.addComponent<ShooterComponent>(_id, {});
    _ecs.registry.addComponent<HealthComponent>(_id, {100, 100});

    // Movement Bindings
    bindActionCallbackPressed("move_left", [this](Registry&, system_context, Entity) {
        this->setVelocity({-200.0f, this->getvelocity().second});
    });
    bindActionCallbackOnReleased("move_left", [this](Registry&, system_context, Entity) {
        this->setVelocity({0.0f, this->getvelocity().second});
    });

    bindActionCallbackPressed("move_right", [this](Registry&, system_context, Entity) {
        this->setVelocity({200.0f, this->getvelocity().second});
    });
    bindActionCallbackOnReleased("move_right", [this](Registry&, system_context, Entity) {
        this->setVelocity({0.0f, this->getvelocity().second});
    });

    bindActionCallbackPressed("move_up", [this](Registry&, system_context, Entity) {
        this->setVelocity({this->getvelocity().first, -200.0f});
    });
    bindActionCallbackOnReleased(
        "move_up", [this](Registry&, system_context, Entity) { this->setVelocity({this->getvelocity().first, 0.0f}); });

    bindActionCallbackPressed("move_down", [this](Registry&, system_context, Entity) {
        this->setVelocity({this->getvelocity().first, 200.0f});
    });
    bindActionCallbackOnReleased("move_down", [this](Registry&, system_context, Entity) {
        this->setVelocity({this->getvelocity().first, 0.0f});
    });

    // Shooting
    bindActionCallbackPressed("shoot", [this](Registry& registry, system_context, Entity entity) {
        if (registry.hasComponent<ShooterComponent>(entity)) {
            auto& shoot = registry.getComponent<ShooterComponent>(entity);
            shoot.is_shooting = true;
            shoot.trigger_pressed = true;
        }
    });
    bindActionCallbackOnReleased("shoot", [this](Registry& registry, system_context, Entity entity) {
        if (registry.hasComponent<ShooterComponent>(entity)) {
            auto& shoot = registry.getComponent<ShooterComponent>(entity);
            shoot.trigger_pressed = false;
            // Note: is_shooting will be handled by ShooterSystem for charged shots
            // For normal shots, we might want to stop immediately, but let's leave it to the system logic
            // which checks trigger_pressed for charging logic.
            // However, if not charging, we should stop shooting.
            // The ShooterSystem logic I wrote earlier handles this:
            // if (!shooter.trigger_pressed && charged.is_charging) -> release charged shot
            // if (!shooter.trigger_pressed && !charged.is_charging) -> stop shooting (eventually)
        }
    });

    BoxCollisionComponent player_collision;
    player_collision.tagCollision.push_back("ENEMY_PROJECTILE");
    player_collision.tagCollision.push_back("AI");
    player_collision.tagCollision.push_back("OBSTACLE");
    _ecs.registry.addComponent<BoxCollisionComponent>(_id, player_collision);
}

void Player::setProjectileType(ShooterComponent::ProjectileType type) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.type = type;
    return;
}

ShooterComponent::ProjectileType Player::getProjectileType() {
    const ShooterComponent& comp = _ecs.registry.getConstComponent<ShooterComponent>(_id);

    return comp.type;
}

void Player::setShootingState(bool state) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.is_shooting = state;
    return;
}

bool Player::isShooting() {
    const ShooterComponent& comp = _ecs.registry.getConstComponent<ShooterComponent>(_id);

    return comp.is_shooting;
}

void Player::setFireRate(double fire_rate) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.fire_rate = fire_rate;
    return;
}

double Player::getFireRate() {
    const ShooterComponent& comp = _ecs.registry.getConstComponent<ShooterComponent>(_id);

    return comp.fire_rate;
}

void Player::setTeam(TeamComponent::Team team) {
    TeamComponent& comp = _ecs.registry.getComponent<TeamComponent>(_id);

    comp.team = team;
    return;
}

TeamComponent::Team Player::getTeam() {
    const TeamComponent& comp = _ecs.registry.getConstComponent<TeamComponent>(_id);

    return comp.team;
}

void Player::setLifePoint(int lifePoint) {
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);

    comp.max_hp = lifePoint;
    comp.current_hp = lifePoint;
    return;
}

int Player::getCurrentHealth() {
    const HealthComponent& comp = _ecs.registry.getConstComponent<HealthComponent>(_id);

    return comp.current_hp;
}

int Player::getMaxHealth() {
    const HealthComponent& comp = _ecs.registry.getConstComponent<HealthComponent>(_id);

    return comp.max_hp;
}

void Player::setCurrentHealth(int health) {
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);
    comp.current_hp = health;
}

void Player::takeDamage(int damage) {
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);

    comp.current_hp -= damage;
    if (comp.current_hp < 0) {
        comp.current_hp = 0;
    }
}
