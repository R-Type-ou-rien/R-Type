#include "Player.hpp"

Player::Player(ECS& ecs, std::pair<float, float> pos) : DynamicActor(ecs, true, "PLAYER") {
    ResourceStat lifepoint;
    lifepoint.max = 100;
    lifepoint.current = 100;
    lifepoint.regenRate = 0;

    addResourceStat("lifepoint", lifepoint);
    setPosition(pos);
    _ecs.registry.addComponent<TeamComponent>(_id, {TeamComponent::Team::ALLY});
    _ecs.registry.addComponent<ShooterComponent>(_id, {});
}

void Player::setProjectileType(ShooterComponent::ProjectileType type) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.type = type;
    return;
}

ShooterComponent::ProjectileType Player::getProjectileType() {
    ShooterComponent comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    return comp.type;
}

void Player::setShootingState(bool state) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.is_shooting = state;
    return;
}

bool Player::isShooting() {
    ShooterComponent comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    return comp.is_shooting;
}

void Player::setFireRate(double fire_rate) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.fire_rate = fire_rate;
    return;
}

double Player::getFireRate() {
    ShooterComponent comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    return comp.fire_rate;
}

void Player::setTeam(TeamComponent::Team team) {
    TeamComponent& comp = _ecs.registry.getComponent<TeamComponent>(_id);

    comp.team = team;
    return;
}

TeamComponent::Team Player::getTeam() {
    TeamComponent comp = _ecs.registry.getComponent<TeamComponent>(_id);

    return comp.team;
}
