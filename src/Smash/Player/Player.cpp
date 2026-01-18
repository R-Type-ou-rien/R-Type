/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** Player.cpp
*/

#include "Player.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/Sprite/AnimatedSprite2D.hpp"
#include "Components/GravityComponent.hpp"
#include "../Lib/PercentageHealth.hpp"
#include "../Lib/EjectionSystem.hpp"


Player::Player(ECS& ecs, std::pair<float, float> pos, int numberPlayer) : _ecs(ecs), _id(_ecs.registry.createEntity()), _numberPlayer(numberPlayer), _isJumping(false) {
    ecs.registry.addComponent<transform_component_s>(_id, {});
    ecs.registry.addComponent<Velocity2D>(_id, {0, 0});
    ecs.registry.addComponent<GravityComponent>(_id, {});
    ecs.registry.addComponent<PercentageHealth>(_id, {2, 0, 150});
    ecs.registry.addComponent<EjectionComponent>(_id, {{0, 0}, 0, false});
    setPosition(pos);
}

//SPRITE ANIMATION
void Player::setSpriteAnimation(const std::string pathname) {
    AnimatedSprite2D animation;

    //IDLE ANIMATION
    AnimationClip clipIdle;

    clipIdle.frames.push_back({0, 0, 64, 64});
    clipIdle.frames.push_back({64, 0, 64, 64});
    clipIdle.frames.push_back({128, 0, 64, 64});
    clipIdle.frames.push_back({192, 0, 64, 64});
    clipIdle.frames.push_back({256, 0, 64, 64});
    clipIdle.mode = AnimationMode::Loop;
    clipIdle.frameDuration = 0.25f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipIdle.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipIdle.handle = _ecs._textureManager.load(pathname, sf::Texture(pathname));
    }
    animation.animations.emplace("idle", clipIdle);
    //IDLE ANIMATION

    //JUMP ANIMATION
    AnimationClip clipJump;

    clipJump.frames.push_back({64 * 17, 0, 64, 64});
    clipJump.mode = AnimationMode::Loop;
    clipJump.frameDuration = 0.25f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipJump.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipJump.handle = _ecs._textureManager.load(pathname, sf::Texture(pathname));
    }
    animation.animations.emplace("jump", clipJump);
    // JUMP ANIMATION

    // FALL ANIMATION
    AnimationClip clipFall;
    clipFall.frames.push_back({64 * 18, 0, 64, 64});
    clipFall.mode = AnimationMode::Loop;
    clipFall.frameDuration = 0.25f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipFall.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipFall.handle = _ecs._textureManager.load(pathname, sf::Texture(pathname));
    }
    animation.animations.emplace("fall", clipFall);

    //RUN ANIMATION
    AnimationClip clipRun;
    clipRun.frames.push_back({64 * 6, 0, 64, 64});
    clipRun.mode = AnimationMode::Loop;
    clipRun.frameDuration = 0.25f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipRun.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipRun.handle = _ecs._textureManager.load(pathname, sf::Texture(pathname));
    }
    animation.animations.emplace("run", clipRun);
    // RUN ANIMATION

    //ATTACK ANIMATION
    AnimationClip clipAttack;

    clipAttack.frames.push_back({64 * 8, 0, 64, 64});
    clipAttack.frames.push_back({64 * 9, 0, 64, 64});
    clipAttack.frames.push_back({64 * 10, 0, 64, 64});
    clipAttack.mode = AnimationMode::Once;
    clipAttack.frameDuration = 0.2f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipAttack.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipAttack.handle = _ecs._textureManager.load(pathname, sf::Texture(pathname));
    }
    animation.animations.emplace("attack", clipAttack);
    // ATTACK ANIMATION

    //HEAVY ATTACK ANIMATION
    AnimationClip clipHeavyAttack;

    clipHeavyAttack.frames.push_back({64 * 11, 0, 64, 64});
    clipHeavyAttack.frames.push_back({64 * 12, 0, 64, 64});
    clipHeavyAttack.frames.push_back({64 * 13, 0, 64, 64});
    clipHeavyAttack.mode = AnimationMode::Once;
    clipHeavyAttack.frameDuration = 0.3f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipHeavyAttack.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipHeavyAttack.handle = _ecs._textureManager.load(pathname, sf::Texture (pathname));
    }
    animation.animations.emplace("attack_heavy", clipHeavyAttack);
    // HEAVY ATTACK ANIMATION

    //SPECIAL ATTACK ANIMATION
    AnimationClip clipSpecialAttack;
    clipSpecialAttack.frames.push_back({64 * 14, 0, 64, 64});
    clipSpecialAttack.frames.push_back({64 * 15, 0, 64, 64});
    clipSpecialAttack.frames.push_back({64 * 16, 0, 64, 64});
    clipSpecialAttack.mode = AnimationMode::Once;
    clipSpecialAttack.frameDuration = 0.3f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipSpecialAttack.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipSpecialAttack.handle = _ecs._textureManager.load(pathname, sf::Texture (pathname));
    }
    animation.animations.emplace("special_attack", clipSpecialAttack);
    // SPECIAL ATTACK ANIMATION

    //BLOCK ANIMATION
    AnimationClip clipBlock;
    clipBlock.frames.push_back({64 * 7, 0, 64, 64});
    clipBlock.mode = AnimationMode::Loop;
    clipBlock.frameDuration = 0.3f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipBlock.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipBlock.handle = _ecs._textureManager.load(pathname, sf::Texture (pathname));
    }
    animation.animations.emplace("block", clipBlock);
    // BLOCK ANIMATION

    // EJECTION ANIMATION
    AnimationClip clipEjection;
    clipEjection.frames.push_back({64 * 20, 0, 64, 64});
    clipEjection.mode = AnimationMode::Loop;
    clipEjection.frameDuration = 0.3f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipEjection.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {
        clipEjection.handle = _ecs._textureManager.load(pathname, sf::Texture (pathname));
    }
    animation.animations.emplace("ejection", clipEjection);
    // EJECTION ANIMATION

    // TAKE DAMAGE ANIMATION
    AnimationClip clipTakeDamage;
    clipTakeDamage.frames.push_back({64 * 19, 0, 64, 64});
    clipTakeDamage.mode = AnimationMode::Loop;
    clipTakeDamage.frameDuration = 0.3f;
    if (_ecs._textureManager.is_loaded(pathname)) {
        clipTakeDamage.handle = _ecs._textureManager.get_handle(pathname).value();
    } else {    
        clipTakeDamage.handle = _ecs._textureManager.load(pathname, sf::Texture (pathname));
    }
    animation.animations.emplace("take_damage", clipTakeDamage);
    // TAKE DAMAGE ANIMATION

    animation.currentAnimation = "idle";
    animation.layer = RenderLayer::Midground;

    _ecs.registry.addComponent<AnimatedSprite2D>(_id, animation);
    return;
}

void Player::flipXSprite(bool flip) {
    AnimatedSprite2D& sprite = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
    sprite.flipX = flip;
}
//SPRITE ANIMATION

void Player::setPosition(std::pair<float, float> pos) {
    transform_component_s& comp = _ecs.registry.getComponent<transform_component_s>(_id);

    _startPos = pos;
    comp.x = pos.first;
    comp.y = pos.second;
    return;
}

void Player::reset() {
    this->setPosition(_startPos);
    this->resetVelocity();
    this->resetGravity();
    this->resetAnimation();
    
    if (_ecs.registry.hasComponent<PercentageHealth>(_id)) {
        auto& health = _ecs.registry.getComponent<PercentageHealth>(_id);
        health.percent = 0;
    }
    return;
}

void Player::resetVelocity() {
    Velocity2D& vel = _ecs.registry.getComponent<Velocity2D>(_id);

    vel.vx = 0.0f;
    vel.vy = 0.0f;
    return;
}

void Player::resetGravity() {
    GravityComponent& grav = _ecs.registry.getComponent<GravityComponent>(_id);

    grav.vectorY = 0.0f;
    _gravityAccumulate = 0.0f;
    return;
}

void Player::resetAnimation() {
    AnimatedSprite2D& sprite = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
    sprite.currentAnimation = "idle";
    sprite.playIfNotPlaying("idle");
    return;
}


void Player::checkMove() {
    Velocity2D& vel = _ecs.registry.getComponent<Velocity2D>(_id);
    GravityComponent& grav = _ecs.registry.getComponent<GravityComponent>(_id);
    AnimatedSprite2D& sprite = _ecs.registry.getComponent<AnimatedSprite2D>(_id);

    if (_ecs.registry.hasComponent<EjectionComponent>(_id)) {
        if (_ecs.registry.getComponent<EjectionComponent>(_id).duration > 0) {
            return;
        }
    }

    if (_numberPlayer == 1) {
        if (grav.grounded) _isJumping = false;

        // Player 1 controls
        if (_ecs.input.isPressed("move_left") && _ecs.input.isPressed("move_right")) {
            vel.vx = 0;
        } else if (_ecs.input.isPressed("move_left")) {
            vel.vx = -200.0f;
            this->flipXSprite(true);
        } else if (_ecs.input.isPressed("move_right")) {
            vel.vx = 200.0f;
            this->flipXSprite(false);
        } else {
            vel.vx = 0;
        }
        if (grav.grounded) {
             _jumpCount = 0;
        } else if (_jumpCount == 0) {
             _jumpCount = 1;
        }

        if (_ecs.input.isJustPressed("jump") && (grav.grounded || _jumpCount < 2)) {
            grav.vectorY = 0;
            vel.vy = -350.0f;
            _isJumping = true;
            grav.grounded = false;
            _jumpCount++;
        } else if (grav.grounded) {
             vel.vy = 0;
        }
    } else if (_numberPlayer == 2) {
        if (grav.grounded) _isJumping = false;

        // Player 2 controls
        if (_ecs.input.isPressed("move_left2") && _ecs.input.isPressed("move_right2")) {
            vel.vx = 0;
        } else if (_ecs.input.isPressed("move_left2")) {
            vel.vx = -200.0f;
            this->flipXSprite(true);
        } else if (_ecs.input.isPressed("move_right2")) {
            vel.vx = 200.0f;
            this->flipXSprite(false);
        } else {
            vel.vx = 0;
        }
        if (grav.grounded) {
             _jumpCount = 0;
        } else if (_jumpCount == 0) {
             _jumpCount = 1;
        }
        if (_ecs.input.isJustPressed("jump2") && (grav.grounded || _jumpCount < 2)) {
            grav.vectorY = 0;
            vel.vy = -350.0f;
            _isJumping = true;
            grav.grounded = false;
            _jumpCount++;
        } else if (grav.grounded) {
            vel.vy = 0;
        }
    }
}

void Player::updateAnimation() {
    Velocity2D& vel = _ecs.registry.getComponent<Velocity2D>(_id);
    GravityComponent& grav = _ecs.registry.getComponent<GravityComponent>(_id);
    AnimatedSprite2D& sprite = _ecs.registry.getComponent<AnimatedSprite2D>(_id);
    
    if (_ecs.registry.hasComponent<EjectionComponent>(_id)) {
        if (_ecs.registry.getComponent<EjectionComponent>(_id).duration > 0) {
            sprite.playIfNotPlaying("ejection");
            return;
        }
    }
    
    if (_numberPlayer == 1) {
        if (!grav.grounded) {
            _gravityAccumulate += grav.vectorY;
        } else {
            _gravityAccumulate = 0.0f;
        }
        if (sprite.isPlaying() && (sprite.currentAnimation == "attack" || sprite.currentAnimation == "attack_heavy" || sprite.currentAnimation == "special_attack")) {
            return;
        }
        if (_ecs.input.isPressed("attack_simple")) {
            sprite.playIfNotPlaying("attack");
            return;
        } 
        else if (_ecs.input.isPressed("attack_heavy")) {
            sprite.playIfNotPlaying("attack_heavy");
            return;
        } 
        else if (_ecs.input.isPressed("special_attack")) {
            sprite.playIfNotPlaying("special_attack");
            return;
        }
        if (_ecs.input.isPressed("block")) {
            sprite.playIfNotPlaying("block");
            return;
        }
        if (_isJumping && !grav.grounded && vel.vy < 0 && vel.vy * -1 / 2 > _gravityAccumulate) {
            sprite.playIfNotPlaying("jump");
        } else if (!grav.grounded) {
            sprite.playIfNotPlaying("fall");
        } else if (vel.vx != 0 && grav.grounded) {
            sprite.playIfNotPlaying("run");
        } else if (vel.vx == 0 && grav.grounded) {
            sprite.playIfNotPlaying("idle");
        } else {
            sprite.playIfNotPlaying("idle");
        }
    } else if (_numberPlayer == 2) {
        if (!grav.grounded) {
            _gravityAccumulate += grav.vectorY;
        } else {
            _gravityAccumulate = 0.0f;
        }
        if (sprite.isPlaying() && (sprite.currentAnimation == "attack" || sprite.currentAnimation == "attack_heavy" || sprite.currentAnimation == "special_attack")) {
            return;
        }
        if (_ecs.input.isPressed("attack_simple2")) {
            sprite.playIfNotPlaying("attack");
            return;
        } 
        else if (_ecs.input.isPressed("attack_heavy2")) {
            sprite.playIfNotPlaying("attack_heavy");
            return;
        } 
        else if (_ecs.input.isPressed("special_attack2")) {
            sprite.playIfNotPlaying("special_attack");
            return;
        }
        else if (_ecs.input.isPressed("block2")) {
            sprite.playIfNotPlaying("block");
            return;
        }
        if (_isJumping && !grav.grounded && vel.vy < 0 && vel.vy * -1 / 2 > _gravityAccumulate) {
            sprite.playIfNotPlaying("jump");
        } else if (!grav.grounded) {
            sprite.playIfNotPlaying("fall");
        } else if (vel.vx != 0 && grav.grounded) {
            sprite.playIfNotPlaying("run");
        } else if (vel.vx == 0 && grav.grounded) {
            sprite.playIfNotPlaying("idle");
        } else {
            sprite.playIfNotPlaying("idle");
        }
    }
}

bool Player::OutOfMap(float xMin, float yMin, float xMax, float yMax) {
    transform_component_s& comp = _ecs.registry.getComponent<transform_component_s>(_id);

    if (comp.x < xMin || comp.x > xMax || comp.y < yMin || comp.y > yMax) {
        return true;
    }
    return false;
}

void Player::setPlayable(bool playable) {
    _playable = playable;
}

float Player::getCurrentHealthPercentage() {
    PercentageHealth& health = _ecs.registry.getComponent<PercentageHealth>(_id);
    return health.percent;
}

Entity Player::getId() const {
    return _id;
}

void Player::updateCooldown(float dt) {
    if (_projectileCooldown > 0) {
        _projectileCooldown -= dt;
    }
}

bool Player::canShoot() {
    return _projectileCooldown <= 0;
}

void Player::resetCooldown() {
    _projectileCooldown = 0.4f;
}