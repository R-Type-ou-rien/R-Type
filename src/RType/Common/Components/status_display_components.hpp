/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Status Display Components - Flexible and configurable UI components
*/

#pragma once

#include <string>
#include "../Components/config.hpp"

namespace StatusPlayerUI {
constexpr float MARGIN_RATIO = 0.02f;
constexpr float VERTICAL_DIVISIONS = 5.0f;
constexpr float HORIZONTAL_DIVISIONS = 3.0f;
constexpr int MAX_LIVES = 5;
constexpr int DIGIT_COUNT = 7;
constexpr float MAX_CHARGE = 1.0f;
constexpr float CHARGE_BAR_HEIGHT = 20.0f;
constexpr float ICON_SIZE = 32.0f;
constexpr float ICON_SPACING = 40.0f;
}  // namespace StatusPlayerUI

class UIStatus {
   public:
    UIStatus(float width, float height) : _windowWidth(width), _windowHeight(height) {}

    float getWindowWidth() const { return _windowWidth; }

    float getWindowHeight() const { return _windowHeight; }

    float getStatusBarY() const { return _windowHeight * 4.0f / StatusPlayerUI::VERTICAL_DIVISIONS; }

    float getStatusBarHeight() const { return _windowHeight / StatusPlayerUI::VERTICAL_DIVISIONS; }

    float getMargin() const { return _windowWidth * StatusPlayerUI::MARGIN_RATIO; }

    float getColumnWidth() const {
        float totalMargins = getMargin() * 4.0f;
        return (_windowWidth - totalMargins) / StatusPlayerUI::HORIZONTAL_DIVISIONS;
    }

    float getColumnX(int column) const {
        float margin = getMargin();
        float colWidth = getColumnWidth();
        return margin + column * (colWidth + margin);
    }

    float getLivesX() const { return getColumnX(0); }
    float getLivesY() const { return getStatusBarY() + getStatusBarHeight() / 2.0f; }

    float getChargeBarX() const { return getColumnX(1); }

    float getChargeBarY() const { return getStatusBarY() + getStatusBarHeight() / 2.0f; }

    float getChargeBarWidth() const { return getColumnWidth(); }

    float getScoreX() const { return getColumnX(2); }
    float getScoreY() const { return getStatusBarY() + getStatusBarHeight() / 2.0f; }

   private:
    float _windowWidth;
    float _windowHeight;
};

class UIComponent {
   public:
    UIComponent() = default;
    UIComponent(float x, float y) : _x(x), _y(y) {}
    virtual ~UIComponent() = default;

    float getX() const { return _x; }

    float getY() const { return _y; }

    void setX(float x) { _x = x; }

    void setY(float y) { _y = y; }
    void setPosition(float x, float y) {
        _x = x;
        _y = y;
    }

   protected:
    float _x = 0.0f;
    float _y = 0.0f;
};

class StatusDisplayComponent {
   public:
    static constexpr auto name = "StatusDisplayComponent";

    StatusDisplayComponent() = default;

    Entity getPlayerEntity() const { return _playerEntity; }

    void setPlayerEntity(Entity entity) { _playerEntity = entity; }

    bool isInitialized() const { return _isInitialized; }

    void setInitialized(bool initialized) { _isInitialized = initialized; }

   private:
    Entity _playerEntity = -1;
    bool _isInitialized = false;
};

class LivesDisplayComponent : public UIComponent {
   public:
    static constexpr auto name = "LivesDisplayComponent";

    LivesDisplayComponent() = default;

    int getCurrentLives() const { return _currentLives; }

    void setCurrentLives(int lives) { _currentLives = lives; }

    int getMaxLives() const { return _maxLives; }

    void setMaxLives(int maxLives) { _maxLives = maxLives; }

    float getIconSize() const { return _iconSize; }

    void setIconSize(float size) { _iconSize = size; }

    float getIconSpacing() const { return _iconSpacing; }

    void setIconSpacing(float spacing) { _iconSpacing = spacing; }

    static LivesDisplayComponent fromLayout(const UIStatus& layout) {
        LivesDisplayComponent component;
        component.setPosition(layout.getLivesX(), layout.getLivesY());
        return component;
    }

    static LivesDisplayComponent fromConfig(const UIConfig::Element& element, const UIStatus& layout) {
        LivesDisplayComponent component;
        component.setX(element.x != 0.0f ? element.x : layout.getLivesX());
        component.setY(element.y != 0.0f ? element.y : layout.getLivesY());
        component.setIconSize(element.icon_size != 0.0f ? element.icon_size : StatusPlayerUI::ICON_SIZE);
        component.setIconSpacing(element.icon_spacing != 0.0f ? element.icon_spacing : StatusPlayerUI::ICON_SPACING);
        return component;
    }

   private:
    int _currentLives = StatusPlayerUI::MAX_LIVES;
    int _maxLives = StatusPlayerUI::MAX_LIVES;
    float _iconSize = StatusPlayerUI::ICON_SIZE;
    float _iconSpacing = StatusPlayerUI::ICON_SPACING;
};

class ChargeBarComponent : public UIComponent {
   public:
    static constexpr auto name = "ChargeBarComponent";

    ChargeBarComponent() = default;

    float getCurrentCharge() const { return _currentCharge; }

    void setCurrentCharge(float charge) { _currentCharge = charge; }

    float getMaxCharge() const { return _maxCharge; }

    void setMaxCharge(float maxCharge) { _maxCharge = maxCharge; }

    float getBarWidth() const { return _barWidth; }

    void setBarWidth(float width) { _barWidth = width; }

    float getBarHeight() const { return _barHeight; }

    void setBarHeight(float height) { _barHeight = height; }

    static ChargeBarComponent fromLayout(const UIStatus& layout) {
        ChargeBarComponent component;
        component.setPosition(layout.getChargeBarX(), layout.getChargeBarY());
        component.setBarWidth(layout.getChargeBarWidth());
        return component;
    }

    static ChargeBarComponent fromConfig(const UIConfig::Element& element, const UIStatus& layout) {
        ChargeBarComponent component;
        if (element.size != 0.0f) {
            component.setPosition(element.x, element.y);
            component.setBarWidth(element.width);
            component.setBarHeight(element.height);
        } else {
            component.setPosition(layout.getChargeBarX(), layout.getChargeBarY());
            component.setBarWidth(layout.getChargeBarWidth());
        }
        return component;
    }

   private:
    float _currentCharge = 0.0f;
    float _maxCharge = StatusPlayerUI::MAX_CHARGE;
    float _barWidth = 0.0f;
    float _barHeight = StatusPlayerUI::CHARGE_BAR_HEIGHT;
};

class ScoreDisplayComponent : public UIComponent {
   public:
    static constexpr auto name = "ScoreDisplayComponent";

    ScoreDisplayComponent() = default;

    int getCurrentScore() const { return _currentScore; }

    void setCurrentScore(int score) { _currentScore = score; }

    int getDigitCount() const { return _digitCount; }

    void setDigitCount(int count) { _digitCount = count; }

    static ScoreDisplayComponent fromLayout(const UIStatus& layout) {
        ScoreDisplayComponent component;
        component.setPosition(layout.getScoreX(), layout.getScoreY());
        return component;
    }

    static ScoreDisplayComponent fromConfig(const UIConfig::Element& element, const UIStatus& layout) {
        ScoreDisplayComponent component;
        component.setX(element.x != 0.0f ? element.x : layout.getScoreX());
        component.setY(element.y != 0.0f ? element.y : layout.getScoreY());
        component.setDigitCount(element.digit_count != 0 ? element.digit_count : StatusPlayerUI::DIGIT_COUNT);
        return component;
    }

   private:
    int _currentScore = 0;
    int _digitCount = StatusPlayerUI::DIGIT_COUNT;
};

class StatusDisplayFactory {
   public:
    static ChargeBarComponent createChargeBar(const UIStatus& layout, const UIConfig& config,
                                              const std::string& elementName = "ChargeBar") {
        auto it = config.elements.find(elementName);
        if (it != config.elements.end()) {
            return ChargeBarComponent::fromConfig(it->second, layout);
        }
        return ChargeBarComponent::fromLayout(layout);
    }

    static ChargeBarComponent createChargeBar(const UIStatus& layout) { return ChargeBarComponent::fromLayout(layout); }

    static LivesDisplayComponent createLivesDisplay(const UIStatus& layout, const UIConfig& config,
                                                    const std::string& elementName = "LivesDisplay") {
        auto it = config.elements.find(elementName);
        if (it != config.elements.end()) {
            return LivesDisplayComponent::fromConfig(it->second, layout);
        }
        return LivesDisplayComponent::fromLayout(layout);
    }

    static LivesDisplayComponent createLivesDisplay(const UIStatus& layout) {
        return LivesDisplayComponent::fromLayout(layout);
    }

    static ScoreDisplayComponent createScoreDisplay(const UIStatus& layout, const UIConfig& config,
                                                    const std::string& elementName = "ScoreDisplay") {
        auto it = config.elements.find(elementName);
        if (it != config.elements.end()) {
            return ScoreDisplayComponent::fromConfig(it->second, layout);
        }
        return ScoreDisplayComponent::fromLayout(layout);
    }

    static ScoreDisplayComponent createScoreDisplay(const UIStatus& layout) {
        return ScoreDisplayComponent::fromLayout(layout);
    }

    static StatusDisplayComponent createStatusDisplay() {
        StatusDisplayComponent component;
        component.setInitialized(true);
        component.setPlayerEntity(-1);
        return component;
    }
};
