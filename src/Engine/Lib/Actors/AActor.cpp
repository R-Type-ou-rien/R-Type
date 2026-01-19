#include "AActor.hpp"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "Components/StandardComponents.hpp"
#include "Components/Sprite/Sprite2D.hpp"
#include "Components/Sprite/AnimatedSprite2D.hpp"

#include "ResourceConfig.hpp"
#include "registry.hpp"

AActor::AActor(ECS& ecs, ResourceManager<TextureAsset>& textures, const std::string name)
    : _ecs(ecs), _textures(textures), _id(_ecs.registry.createEntity()) {
    std::vector<std::string> tag_init;
    BoxCollisionComponent collision;

    tag_init.insert(tag_init.cbegin(), name);
    _ecs.registry.addComponent<TagComponent>(_id, {tag_init});
    _ecs.registry.addComponent<TransformComponent>(_id, {0, 0});
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
    return _ecs.registry.getConstComponent<TagComponent>(_id).tags;
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
    TransformComponent comp = _ecs.registry.getConstComponent<TransformComponent>(_id);

    return std::pair<float, float>(comp.x, comp.y);
}

void AActor::setPosition(std::pair<float, float> pos) {
    TransformComponent& comp = _ecs.registry.getComponent<TransformComponent>(_id);

    comp.x = pos.first;
    comp.y = pos.second;
    return;
}

float AActor::getRotation() {
    TransformComponent comp = _ecs.registry.getConstComponent<TransformComponent>(_id);

    return comp.rotation;
}

void AActor::setRotation(float rotation) {
    TransformComponent& comp = _ecs.registry.getComponent<TransformComponent>(_id);

    comp.rotation = rotation;
    return;
}

void AActor::setScale(std::pair<float, float> scale) {
    TransformComponent& comp = _ecs.registry.getComponent<TransformComponent>(_id);

    comp.scale_x = scale.first;
    comp.scale_y = scale.second;
    return;
}

std::pair<float, float> AActor::getScale() {
    TransformComponent comp = _ecs.registry.getConstComponent<TransformComponent>(_id);

    return std::pair<float, float>(comp.scale_x, comp.scale_y);
}

void AActor::setTextureEnemy(const std::string pathname) {
    AnimatedSprite2D animation;

    AnimationClip clip;
    clip.frameDuration = 0.6f;
    clip.mode = AnimationMode::Loop;

    for (float i = 0; i < 8; i++)
        clip.frames.emplace_back(i * 32, 0, 34, 34);
    for (float i = 0; i < 8; i++)
        clip.frames.emplace_back(i * 32, 34, 34, 34);

    if (_textures.is_loaded(pathname)) {
        clip.handle = _textures.get_handle(pathname).value();
    } else {
        clip.handle = _textures.load(pathname, TextureAsset(pathname));
    }
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";
    _ecs.registry.addComponent<AnimatedSprite2D>(_id, animation);
    return;
}

// void AActor::setTextureEnemy(const std::string pathname) {
//     Sprite2DComponent sprite;
//     sprite.animation_speed = 0.6f;
//     sprite.current_animation_frame = 0;
//     sprite.is_animated = true;
//     sprite.loop_animation = true;
//     for (float i = 0; i < 8; i++)
//         sprite.frames.push_back({i * 32, 0, 34, 34});
//     for (float i = 0; i < 8; i++)
//         sprite.frames.push_back({i * 32, 34, 34, 34});

//     if (_textures.is_loaded(pathname)) {
//         sprite.handle = _textures.get_handle(pathname).value();
//     } else {
//         sprite.handle = _textures.load(pathname, TextureAsset(pathname));
//     }
//     _ecs.registry.addComponent<Sprite2DComponent>(_id, sprite);
//     return;
// }

void AActor::setTextureBoss(const std::string pathname) {
    AnimatedSprite2D animation;

    AnimationClip clip;
    clip.frameDuration = 0.5f;
    clip.mode = AnimationMode::Loop;

    for (float i = 0; i < 4; i++) {
        clip.frames.emplace_back(260, i * 143, 260, 143);
        clip.frames.emplace_back(0, i * 143, 260, 143);
    }

    if (_textures.is_loaded(pathname)) {
        clip.handle = _textures.get_handle(pathname).value();
    } else {
        clip.handle = _textures.load(pathname, TextureAsset(pathname));
    }
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";
    _ecs.registry.addComponent<AnimatedSprite2D>(_id, animation);
    return;
}

// void AActor::setTextureBoss(const std::string pathname) {
//     Sprite2DComponent sprite;
//     sprite.animation_speed = 0.5f;
//     sprite.current_animation_frame = 0;
//     sprite.is_animated = true;
//     sprite.loop_animation = false;
//     for (float i = 0; i < 4; i++) {
//         sprite.frames.push_back({260, i * 143, 260, 143});
//         sprite.frames.push_back({0, i * 143, 260, 143});
//     }

//     if (_textures.is_loaded(pathname)) {
//         sprite.handle = _textures.get_handle(pathname).value();
//     } else {
//         sprite.handle = _textures.load(pathname, TextureAsset(pathname));
//     }
//     _ecs.registry.addComponent<Sprite2DComponent>(_id, sprite);
//     return;
// }

void AActor::setTexture(const std::string pathname) {
    AnimatedSprite2D animation;

    AnimationClip clip;
    clip.frameDuration = 0;
    clip.mode = AnimationMode::Loop;

    if (_textures.is_loaded(pathname)) {
        clip.handle = _textures.get_handle(pathname).value();
    } else {
        clip.handle = _textures.load(pathname, TextureAsset(pathname));
    }
    animation.animations.emplace("idle", clip);
    animation.currentAnimation = "idle";

    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        comp = animation;
    } else {
        _ecs.registry.addComponent<AnimatedSprite2D>(_id, animation);
    }
    return;
}

// void AActor::setTexture(const std::string pathname) {
//     Sprite2DComponent sprite;
//     sprite.animation_speed = 0;
//     sprite.current_animation_frame = 0;
//     sprite.dimension = {0, 0, 0, 0};
//     sprite.z_index = 0;

//     if (_textures.is_loaded(pathname)) {
//         sprite.handle = _textures.get_handle(pathname).value();
//     } else {
//         sprite.handle = _textures.load(pathname, TextureAsset(pathname));
//     }

//     if (_ecs.registry.hasComponent<Sprite2DComponent>(_id)) {
//         Sprite2DComponent& comp = _ecs.registry.getComponent<Sprite2DComponent>(_id);
//         comp = sprite;
//     } else {
//         _ecs.registry.addComponent<Sprite2DComponent>(_id, sprite);
//     }
//     return;
// }

void AActor::setTextureDimension(rect dimension) {
    // Sprite2DComponent& comp = _ecs.registry.getComponent<Sprite2DComponent>(_id);
    // comp.dimension = dimension;
    // return;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        if (comp.animations.at(comp.currentAnimation).frames.empty()) {
            comp.animations.at(comp.currentAnimation)
                .frames.emplace_back(static_cast<int>(dimension.x), static_cast<int>(dimension.y),
                                     static_cast<int>(dimension.width), static_cast<int>(dimension.height));
            return;
        }
        comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).width =
            static_cast<int>(dimension.width);
        comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).height =
            static_cast<int>(dimension.height);
        comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).x = static_cast<int>(dimension.x);
        comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).y = static_cast<int>(dimension.y);
        return;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    comp.rect.width = static_cast<int>(dimension.width);
    comp.rect.height = static_cast<int>(dimension.height);
    comp.rect.x = static_cast<int>(dimension.y);
    comp.rect.y = static_cast<int>(dimension.y);
    return;
}

rect AActor::getDimension() {
    // Sprite2DComponent comp = _ecs.registry.getConstComponent<Sprite2DComponent>(_id);

    // return comp.dimension;
    rect dimension;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        dimension.width =
            static_cast<float>(comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).width);
        dimension.height =
            static_cast<float>(comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).height);
        dimension.x = static_cast<float>(comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).x);
        dimension.y = static_cast<float>(comp.animations.at(comp.currentAnimation).frames.at(comp.currentFrameIndex).y);
        return dimension;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    dimension.width = static_cast<float>(comp.rect.width);
    dimension.height = static_cast<float>(comp.rect.height);
    dimension.x = static_cast<float>(comp.rect.x);
    dimension.y = static_cast<float>(comp.rect.y);
    return dimension;
}

void AActor::setAnimation(bool state) {
    // Sprite2DComponent& comp = _ecs.registry.getComponent<Sprite2DComponent>(_id);

    // comp.is_animated = state;
    // return;
    // Cant change with the new component
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        return;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    return;
}

bool AActor::isAnimated() {
    // Sprite2DComponent comp = _ecs.registry.getConstComponent<Sprite2DComponent>(_id);

    // return comp.is_animated;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        return true;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    return false;
}

void AActor::setAnimationSpeed(float speed) {
    // Sprite2DComponent& comp = _ecs.registry.getComponent<Sprite2DComponent>(_id);

    // comp.animation_speed = speed;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        comp.animations.at(comp.currentAnimation).frameDuration = speed;
        return;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    return;
}

float AActor::getAnimationSpeed() {
    // Sprite2DComponent comp = _ecs.registry.getConstComponent<Sprite2DComponent>(_id);

    // return comp.animation_speed;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        return comp.animations.at(comp.currentAnimation).frameDuration;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    return 0.f;
}

void AActor::setDisplayLayer(int layer) {
    // Sprite2DComponent& comp = _ecs.registry.getComponent<Sprite2DComponent>(_id);

    // comp.z_index = layer;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        comp.layer = static_cast<RenderLayer>(layer);
        return;
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    comp.layer = static_cast<RenderLayer>(layer);
    return;
}

int AActor::getDisplayLayer() {
    // Sprite2DComponent comp = _ecs.registry.getConstComponent<Sprite2DComponent>(_id);

    // return comp.z_index;
    if (_ecs.registry.hasComponent<AnimatedSprite2D>(_id)) {
        AnimatedSprite2D& comp = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
        return static_cast<int>(comp.layer);
    }
    Sprite2D& comp = _ecs.registry.getComponent<Sprite2D>(_id);
    return static_cast<int>(comp.layer);
}

void AActor::setCollisionTags(std::vector<std::string> tags) {
    BoxCollisionComponent& comp = _ecs.registry.getComponent<BoxCollisionComponent>(_id);

    comp.tagCollision = tags;
    return;
}

void AActor::addCollisionTag(const std::string tag) {
    std::vector<std::string>& tags = _ecs.registry.getComponent<BoxCollisionComponent>(_id).tagCollision;

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
