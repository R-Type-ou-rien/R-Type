#include "DynamicActor.hpp"

#include <string>
#include <utility>
#include <vector>

#include "Components/StandardComponents.hpp"

DynamicActor::DynamicActor(ECS& ecs, bool playable, const std::string name) : AActor(ecs, name), _playable(playable) {
    ResourceComponent resources;

    _ecs.registry.addComponent<ResourceComponent>(_id, resources);

    _ecs.registry.addComponent<Velocity2D>(_id, {0, 0});

    _ecs.registry.addComponent<ActionScript>(_id, {});

    if (!_playable)
        _ecs.registry.addComponent<PatternComponent>(_id, {});
}

void DynamicActor::addResourceStat(const std::string res_name, ResourceStat& resource) {
    ResourceComponent& comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    comp.resources.emplace(res_name, resource);
    return;
}

float DynamicActor::getMaxResourceStat(const std::string name) {
    ResourceComponent comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    return comp.resources[name].max;
}

float DynamicActor::getCurrentResourceStat(const std::string name) {
    ResourceComponent comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    return comp.resources[name].current;
}

float DynamicActor::getRegenResourceStat(const std::string name) {
    ResourceComponent comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    return comp.resources[name].regenRate;
}

void DynamicActor::setMaxResourceStat(const std::string res_name, float max) {
    ResourceComponent& comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    comp.resources[res_name].max = max;
    return;
}

void DynamicActor::setCurrentResourceStat(const std::string res_name, float current) {
    ResourceComponent& comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    comp.resources[res_name].current = current;
    return;
}

void DynamicActor::setRegenResourceStat(const std::string res_name, float regen) {
    ResourceComponent& comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    comp.resources[res_name].regenRate = regen;
    return;
}

bool DynamicActor::hasResource(const std::string name) {
    ResourceComponent& comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    return comp.resources.find(name) != comp.resources.end();
}

void DynamicActor::addEmptyEffect(const std::string res_name, std::function<void()> effect) {
    ResourceComponent& comp = _ecs.registry.getComponent<ResourceComponent>(_id);

    if (!hasResource(res_name)) {
        return;
    }
    comp.empty_effects[res_name] = effect;
    return;
}

void DynamicActor::setVelocity(std::pair<float, float> velocity) {
    Velocity2D& comp = _ecs.registry.getComponent<Velocity2D>(_id);

    comp.vx = velocity.first;
    comp.vy = velocity.second;
    return;
}

std::pair<float, float> DynamicActor::getvelocity() {
    Velocity2D comp = _ecs.registry.getComponent<Velocity2D>(_id);

    return std::pair<float, float>(comp.vx, comp.vy);
}

void DynamicActor::bindActionCallbackOnPressed(Action action_name, ActionCallback callback) {
    ActionScript& comp = _ecs.registry.getComponent<ActionScript>(_id);

    comp.actionOnPressed[action_name] = callback;
    return;
}

void DynamicActor::bindActionCallbackPressed(Action action_name, ActionCallback callback) {
    ActionScript& comp = _ecs.registry.getComponent<ActionScript>(_id);

    comp.actionPressed[action_name] = callback;
    return;
}

void DynamicActor::bindActionCallbackOnReleased(Action action_name, ActionCallback callback) {
    ActionScript& comp = _ecs.registry.getComponent<ActionScript>(_id);

    comp.actionOnReleased[action_name] = callback;
    return;
}

void DynamicActor::removeActionCallbackOnPressed(Action action_name) {
    ActionScript& comp = _ecs.registry.getComponent<ActionScript>(_id);

    comp.actionOnPressed.erase(action_name);
    return;
}

void DynamicActor::removeActionCallbackPressed(Action action_name) {
    ActionScript& comp = _ecs.registry.getComponent<ActionScript>(_id);

    comp.actionPressed.erase(action_name);
    return;
}

void DynamicActor::removeActionCallbackOnReleased(Action action_name) {
    ActionScript& comp = _ecs.registry.getComponent<ActionScript>(_id);

    comp.actionOnReleased.erase(action_name);
    return;
}

std::unordered_map<Action, ActionCallback> DynamicActor::getActionCallbackOnPressed() {
    ActionScript comp = _ecs.registry.getComponent<ActionScript>(_id);

    return comp.actionOnPressed;
}

std::unordered_map<Action, ActionCallback> DynamicActor::getActionCallbackPressed() {
    ActionScript comp = _ecs.registry.getComponent<ActionScript>(_id);

    return comp.actionPressed;
}

std::unordered_map<Action, ActionCallback> DynamicActor::getActionCallbackOnReleased() {
    ActionScript comp = _ecs.registry.getComponent<ActionScript>(_id);

    return comp.actionOnReleased;
}

void DynamicActor::setPattern(std::vector<std::pair<float, float>> way_points)
{
    if (_playable)
        return;
    PatternComponent& comp = _ecs.registry.getComponent<PatternComponent>(_id);

    comp.waypoints = way_points;
    return;
}

void DynamicActor::setPatternType(PatternComponent::PatternType type)
{
    if (_playable)
        return;
    PatternComponent& comp = _ecs.registry.getComponent<PatternComponent>(_id);

    comp.type = type;
    return;
}

std::vector<std::pair<float, float>> DynamicActor::getPattern()

{
    if (_playable)
        return std::vector<std::pair<float, float>>();
    PatternComponent comp = _ecs.registry.getComponent<PatternComponent>(_id);

    return comp.waypoints;
}

void DynamicActor::setPatternActive(bool state)
{
    if (_playable)
        return;
    PatternComponent& comp = _ecs.registry.getComponent<PatternComponent>(_id);

    comp.is_active = state;
    return;
}

bool DynamicActor::getPatternActiveState()
{
    if (_playable)
        return false;
    PatternComponent comp = _ecs.registry.getComponent<PatternComponent>(_id);

    return comp.is_active;
}

void DynamicActor::setPatternLoop(bool state)
{
    if (_playable)
        return;
    PatternComponent& comp = _ecs.registry.getComponent<PatternComponent>(_id);

    comp.loop = state;
    return;
}

bool DynamicActor::getPatternLoopState()
{
    if (_playable)
        return false;
    PatternComponent comp = _ecs.registry.getComponent<PatternComponent>(_id);

    return comp.loop;
}

void DynamicActor::setPatternSpeed(float speed)
{
    if (_playable)
        return;
    PatternComponent& comp = _ecs.registry.getComponent<PatternComponent>(_id);

    comp.speed = speed;
    return;
}

float DynamicActor::getPatternSpeed()
{
    if (_playable)
        return -1;
    PatternComponent& comp = _ecs.registry.getComponent<PatternComponent>(_id);

    return comp.speed;
}
