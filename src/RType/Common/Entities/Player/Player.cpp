#include <utility>
#include "Player.hpp"
#include "src/RType/Common/Systems/health.hpp"
#include "src/RType/Common/Systems/damage.hpp"
#include "src/RType/Common/Systems/score.hpp"
#include "CollisionSystem.hpp"

Player::Player(ECS& ecs, ResourceManager<TextureAsset>& textures, std::pair<float, float> pos,
               const EntityConfig& config)
    : DynamicActor(ecs, true, textures, "PLAYER") {
    int hp = config.hp.value_or(5);
    float speed = config.speed.value_or(200.0f);

    ResourceStat lifepoint;
    lifepoint.max = static_cast<float>(hp);
    lifepoint.current = static_cast<float>(hp);
    lifepoint.regenRate = 0;

    addResourceStat("lifepoint", lifepoint);
    setPosition(pos);
    _ecs.registry.addComponent<TeamComponent>(_id, {TeamComponent::Team::ALLY});

    ShooterComponent shooter;
    shooter.fire_rate = config.fire_rate.value_or(0.25f);
    _ecs.registry.addComponent<ShooterComponent>(_id, shooter);
    _ecs.registry.addComponent<ScoreComponent>(_id, {0, 0});
    _ecs.registry.addComponent<HealthComponent>(_id, {hp, hp, 0.0f, 1.5f});
    _ecs.registry.addComponent<DamageOnCollision>(_id, {0});

    bindActionCallbackPressed("move_left", [this, speed](Registry&, system_context, Entity) {
        this->setVelocity({-speed, this->getvelocity().second});
    });

    bindActionCallbackOnReleased("move_left", [this](Registry&, system_context, Entity) {
        this->setVelocity({0.0f, this->getvelocity().second});
    });

    bindActionCallbackPressed("move_right", [this, speed](Registry&, system_context, Entity) {
        this->setVelocity({speed, this->getvelocity().second});
    });

    bindActionCallbackOnReleased("move_right", [this](Registry&, system_context, Entity) {
        this->setVelocity({0.0f, this->getvelocity().second});
    });

    bindActionCallbackPressed("move_up", [this, speed](Registry&, system_context, Entity) {
        this->setVelocity({this->getvelocity().first, -speed});
    });

    bindActionCallbackOnReleased(
        "move_up", [this](Registry&, system_context, Entity) { this->setVelocity({this->getvelocity().first, 0.0f}); });

    bindActionCallbackPressed("move_down", [this, speed](Registry&, system_context, Entity) {
        this->setVelocity({this->getvelocity().first, speed});
    });

    bindActionCallbackOnReleased("move_down", [this](Registry&, system_context, Entity) {
        this->setVelocity({this->getvelocity().first, 0.0f});
    });

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

            if (registry.hasComponent<ChargedShotComponent>(entity)) {
                auto& charged = registry.getComponent<ChargedShotComponent>(entity);
                if (!charged.is_charging) {
                    shoot.is_shooting = false;
                }
            } else {
                shoot.is_shooting = false;
            }
        }
    });

    bindActionCallbackPressed("toggle_pod", [this](Registry& registry, system_context, Entity entity) {
        if (registry.hasComponent<PlayerPodComponent>(entity)) {
            auto& player_pod = registry.getComponent<PlayerPodComponent>(entity);
            if (player_pod.has_pod) {
                player_pod.detach_requested = true;
            }
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
