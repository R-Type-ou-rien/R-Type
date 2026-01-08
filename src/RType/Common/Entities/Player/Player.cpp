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

void Player::takeDamage(int damage) {
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);

    comp.current_hp -= damage;
    if (comp.current_hp < 0) {
        comp.current_hp = 0;
    }
}
