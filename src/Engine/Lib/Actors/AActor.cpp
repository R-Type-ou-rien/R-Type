#include "AActor.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "Components/StandardComponents.hpp"
#include "registry.hpp"

AActor::AActor(ECS& ecs, const std::string name) : _ecs(ecs), _id(_ecs.registry.createEntity()) {
    std::vector<std::string> tag_init;
    BoxCollisionComponent collision;

    tag_init.insert(tag_init.cbegin(), name);
    _ecs.registry.addComponent<TagComponent>(_id, {tag_init});

    _ecs.registry.addComponent<transform_component_s>(_id, {0, 0});

    _ecs.registry.addComponent<BoxCollisionComponent>(_id, collision);
    return;
}

AActor::~AActor() {
    _ecs.registry.destroyEntity(_id);
}

Entity AActor::getId() {
    return _id;
}

std::vector<std::string> AActor::getTags() {
    return _ecs.registry.getComponent<TagComponent>(_id).tags;
}

void AActor::setTags(const std::vector<std::string> tags) {
    _ecs.registry.getComponent<TagComponent>(_id).tags = tags;
    return;
}

void AActor::addTag(const std::string tag) {
    std::vector<std::string>& tags = _ecs.registry.getComponent<TagComponent>(_id).tags;
    tags.emplace(tags.cbegin(), tag);
    return;
}

void AActor::removeTag(const std::string tag) {
    std::vector<std::string>& tags = _ecs.registry.getComponent<TagComponent>(_id).tags;
    if (tags.size() == 1 || find(tags.begin(), tags.end(), tag) == tags.cend()) {
        std::cerr << "The tag " << tag << " coudldn't be removed" << std::endl;
        return;
    }
    tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
    return;
}

std::pair<float, float> AActor::getPosition() {
    transform_component_s& comp = _ecs.registry.getComponent<transform_component_s>(_id);

    return std::pair<float, float>(comp.x, comp.y);
}

void AActor::setPosition(std::pair<float, float> pos) {
    transform_component_s& comp = _ecs.registry.getComponent<transform_component_s>(_id);

    comp.x = pos.first;
    comp.y = pos.second;
    return;
}

float AActor::getRotation() {
    transform_component_s comp = _ecs.registry.getComponent<transform_component_s>(_id);

    return comp.rotation;
}

void AActor::setRotation(float rotation) {
    transform_component_s& comp = _ecs.registry.getComponent<transform_component_s>(_id);

    comp.rotation = rotation;
    return;
}

void AActor::setScale(std::pair<float, float> scale) {
    transform_component_s& comp = _ecs.registry.getComponent<transform_component_s>(_id);

    comp.scale_x = scale.first;
    comp.scale_y = scale.second;
    return;
}

std::pair<float, float> AActor::getScale() {
    transform_component_s comp = _ecs.registry.getComponent<transform_component_s>(_id);

    return std::pair<float, float>(comp.scale_x, comp.scale_y);
}

void AActor::setTexture(const std::string pathname) {
    sprite2D_component_s sprite;
    sprite.animation_speed = 0;
    sprite.current_animation_frame = 0;
    sprite.dimension = {0, 0, 0, 0};
    sprite.z_index = 0;

    if (_ecs._textureManager.is_loaded(pathname)) {
        sprite.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        sprite.handle = _ecs._textureManager.load_resource(pathname, sf::Texture(pathname));
    }
    _ecs.registry.addComponent<sprite2D_component_s>(_id, sprite);
    return;
}

void AActor::setTextureDimension(rect dimension) {
    sprite2D_component_s& comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    comp.dimension = dimension;
    return;
}

rect AActor::getDimension() {
    sprite2D_component_s comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    return comp.dimension;
}

void AActor::setAnimation(bool state) {
    sprite2D_component_s& comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    comp.is_animated = state;
    return;
}

bool AActor::isAnimmated() {
    sprite2D_component_s comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    return comp.is_animated;
}

void AActor::setAnimationSpeed(float speed) {
    sprite2D_component_s& comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    comp.animation_speed = speed;
    return;
}

float AActor::getAnimationSpeed() {
    sprite2D_component_s comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    return comp.animation_speed;
}

void AActor::setDisplayLayer(int layer) {
    sprite2D_component_s& comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    comp.z_index = layer;
    return;
}

int AActor::getDisplayLayer() {
    sprite2D_component_s comp = _ecs.registry.getComponent<sprite2D_component_s>(_id);

    return comp.z_index;
}

void AActor::setCollisionTags(std::vector<std::string> tags) {
    BoxCollisionComponent& comp = _ecs.registry.getComponent<BoxCollisionComponent>(_id);

    comp.tagCollision = tags;
    return;
}

void AActor::addCollisionTag(const std::string tag) {
    std::vector<std::string>& tags = _ecs.registry.getComponent<BoxCollisionComponent>(_id).tagCollision;
    ;

    tags.push_back(tag);
    return;
}

void AActor::removeCollisionTag(const std::string tag) {
    std::vector<std::string>& tags = _ecs.registry.getComponent<BoxCollisionComponent>(_id).tagCollision;

    if (find(tags.begin(), tags.end(), tag) == tags.cend()) {
        std::cerr << "The tag " << tag << " coudldn't be removed" << std::endl;
        return;
    }
    tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
    return;
}

void AActor::emptyCollisionTags() {
    std::vector<std::string>& tags = _ecs.registry.getComponent<BoxCollisionComponent>(_id).tagCollision;

    tags.clear();
    return;
}