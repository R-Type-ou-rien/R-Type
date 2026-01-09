#include "UISystem.hpp"
#include <iostream>
#include <SFML/Window/Mouse.hpp>

void UISystem::bindAction(const std::string& actionName, std::function<void()> callback) {
    _actions[actionName] = callback;
}

void UISystem::update(Registry& registry, system_context context) {
    try {
        // Iterate over ButtonComponent entities
        auto& buttonEntities = registry.getEntities<ButtonComponent>();

        for (auto entityID : buttonEntities) {
            Entity entity = (Entity)entityID;

            // Check if it also has ClickableComponent
            if (registry.hasComponent<ClickableComponent>(entity)) {
                auto& button = registry.getComponent<ButtonComponent>(entity);
                auto& clickable = registry.getComponent<ClickableComponent>(entity);
                handleButtonState(registry, entity, button, clickable, context);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[UISystem] Error: " << e.what() << std::endl;
    }
}

void UISystem::handleButtonState(Registry& registry, Entity entity, ButtonComponent& button,
                                 ClickableComponent& clickable, system_context& context) {
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    bool leftClick = false;

#ifdef CLIENT_BUILD
    auto pos = sf::Mouse::getPosition(context.window);
    auto worldPos = context.window.mapPixelToCoords(pos);
    mouseX = worldPos.x;
    mouseY = worldPos.y;

    // SFML 2.x assumption
    leftClick = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
#endif

    float x = clickable.bounds.x;
    float y = clickable.bounds.y;

    if (registry.hasComponent<transform_component_s>(entity)) {
        auto& transform = registry.getComponent<transform_component_s>(entity);
        x += transform.x;
        y += transform.y;
    }

    // Simple AABB
    bool hovered =
        (mouseX >= x && mouseX <= x + clickable.bounds.width && mouseY >= y && mouseY <= y + clickable.bounds.height);

    clickable.isHovered = hovered;

    // State Logic
    if (button.state == ButtonComponent::DISABLED)
        return;

    if (hovered) {
        if (leftClick) {
            button.state = ButtonComponent::PRESSED;
            clickable.isClicked = true;
        } else {
            if (clickable.isClicked) {
                // Click Released -> Trigger Action
                if (button.state == ButtonComponent::PRESSED) {
                    if (_actions.find(button.actionName) != _actions.end()) {
                        std::cout << "[UISystem] Action Triggered: " << button.actionName << std::endl;
                        _actions[button.actionName]();
                    }
                }
                clickable.isClicked = false;
            }
            button.state = ButtonComponent::HOVER;
        }
    } else {
        if (clickable.isClicked && !leftClick) {
            clickable.isClicked = false;
        }
        button.state = ButtonComponent::NORMAL;
    }

    updateButtonVisuals(registry, entity, button);
}

void UISystem::updateButtonVisuals(Registry& registry, Entity entity, ButtonComponent& button) {
    if (registry.hasComponent<sprite2D_component_s>(entity)) {
        auto& sprite = registry.getComponent<sprite2D_component_s>(entity);

        rect targetFrame = button.normalFrame;

        switch (button.state) {
            case ButtonComponent::HOVER:
                targetFrame = button.hoverFrame;
                break;
            case ButtonComponent::PRESSED:
                targetFrame = button.pressedFrame;
                break;
            default:
                break;
        }

        if (!sprite.frames.empty()) {
            sprite.frames[0] = targetFrame;
        } else {
            sprite.frames.push_back(targetFrame);
        }

        sprite.dimension.width = targetFrame.width;
        sprite.dimension.height = targetFrame.height;
    }
}
