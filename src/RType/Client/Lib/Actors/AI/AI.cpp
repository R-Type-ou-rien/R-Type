#include "AI.hpp"
#include "src/RType/Common/Components/damage.hpp"
#include "src/RType/Common/Components/health.hpp"

AI::AI(ECS& ecs, std::pair<float, float> pos) : DynamicActor(ecs, false, "AI") {
    ResourceStat lifepoint;
    lifepoint.max = 100;
    lifepoint.current = 100;
    lifepoint.regenRate = 0;

    addResourceStat("lifepoint", lifepoint);
    setPosition(pos);
    _ecs.registry.addComponent<TeamComponent>(_id, {TeamComponent::Team::ENEMY});
    _ecs.registry.addComponent<ShooterComponent>(_id, {});
    _ecs.registry.addComponent<HealthComponent>(_id, {100, 100});
    _ecs.registry.addComponent<DamageOnCollision>(_id, {10});
}

void AI::setProjectileType(ShooterComponent::ProjectileType type) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.type = type;
    return;
}

ShooterComponent::ProjectileType AI::getProjectileType() {
    ShooterComponent comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    return comp.type;
}

void AI::setShootingState(bool state) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.is_shooting = state;
    return;
}

bool AI::isShooting() {
    ShooterComponent comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    return comp.is_shooting;
}

void AI::setFireRate(double fire_rate) {
    ShooterComponent& comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    comp.fire_rate = fire_rate;
    return;
}

double AI::getFireRate() {
    ShooterComponent comp = _ecs.registry.getComponent<ShooterComponent>(_id);

    return comp.fire_rate;
}

void AI::setTeam(TeamComponent::Team team) {
    TeamComponent& comp = _ecs.registry.getComponent<TeamComponent>(_id);

    comp.team = team;
    return;
}

TeamComponent::Team AI::getTeam() {
    TeamComponent comp = _ecs.registry.getComponent<TeamComponent>(_id);

    return comp.team;
}

void AI::setLifePoint(int lifePoint) {
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);

    comp.max_hp = lifePoint;
    comp.current_hp = lifePoint;
    return;
}
int AI::getCurrentHealth() {
    HealthComponent comp = _ecs.registry.getComponent<HealthComponent>(_id);

    return comp.current_hp;
}

int AI::getMaxHealth() {
    HealthComponent comp = _ecs.registry.getComponent<HealthComponent>(_id);

    return comp.max_hp;
}

void AI::setCurrentHealth(int health) {
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);

    if (health < 0) {
        comp.current_hp = 0;
    } else if (health > comp.max_hp) {
        comp.current_hp = comp.max_hp;
    } else {
        comp.current_hp = health;
    }
}

void AI::takeDamage(int damage)

{
    HealthComponent& comp = _ecs.registry.getComponent<HealthComponent>(_id);

    comp.current_hp -= damage;
    if (comp.current_hp < 0) {
        comp.current_hp = 0;
    }
}
