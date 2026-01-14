/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Status Display Components - Flexible and configurable UI components
*/

#pragma once

#include <string>
#include "../Components/config.hpp"

namespace StatusDisplayDefaults {
    constexpr float STATUS_BAR_Y = 1030.0f;
    constexpr float STATUS_BAR_HEIGHT = 50.0f;
    constexpr float MARGIN = 20.0f;

    namespace ChargeBar {
        constexpr float X = 860.0f;
        constexpr float Y = STATUS_BAR_Y;
        constexpr float WIDTH = 200.0f;
        constexpr float HEIGHT = 20.0f;
        constexpr float MAX_CHARGE = 1.0f;
    }

    namespace Lives {
        constexpr float X = 50.0f;
        constexpr float Y = STATUS_BAR_Y;
        constexpr float ICON_SIZE = 32.0f;
        constexpr float ICON_SPACING = 40.0f;
        constexpr int MAX_LIVES = 3;
    }

    namespace Score {
        constexpr float X = 1650.0f;
        constexpr float Y = STATUS_BAR_Y;
        constexpr int DIGIT_COUNT = 7;
    }
}

struct StatusDisplayComponent {
    static constexpr auto name = "StatusDisplayComponent";

    Entity player_entity = -1;
    bool is_initialized = false;
};

struct ChargeBarComponent {
    static constexpr auto name = "ChargeBarComponent";

    float current_charge = 0.0f;
    float max_charge = StatusDisplayDefaults::ChargeBar::MAX_CHARGE;
    float bar_width = StatusDisplayDefaults::ChargeBar::WIDTH;
    float bar_height = StatusDisplayDefaults::ChargeBar::HEIGHT;
    float x = StatusDisplayDefaults::ChargeBar::X;
    float y = StatusDisplayDefaults::ChargeBar::Y;

    static ChargeBarComponent fromConfig(const UIConfig::Element& element) {
        ChargeBarComponent component;
        component.x = element.x != 0.0f ? element.x : StatusDisplayDefaults::ChargeBar::X;
        component.y = element.y != 0.0f ? element.y : StatusDisplayDefaults::ChargeBar::Y;
        component.bar_width = element.width != 0.0f ? element.width : StatusDisplayDefaults::ChargeBar::WIDTH;
        component.bar_height = element.height != 0.0f ? element.height : StatusDisplayDefaults::ChargeBar::HEIGHT;
        return component;
    }
};

struct LivesDisplayComponent {
    static constexpr auto name = "LivesDisplayComponent";

    int current_lives = StatusDisplayDefaults::Lives::MAX_LIVES;
    int max_lives = StatusDisplayDefaults::Lives::MAX_LIVES;
    float x = StatusDisplayDefaults::Lives::X;
    float y = StatusDisplayDefaults::Lives::Y;
    float icon_size = StatusDisplayDefaults::Lives::ICON_SIZE;
    float icon_spacing = StatusDisplayDefaults::Lives::ICON_SPACING;


    static LivesDisplayComponent fromConfig(const UIConfig::Element& element) {
        LivesDisplayComponent component;
        component.x = element.x != 0.0f ? element.x : StatusDisplayDefaults::Lives::X;
        component.y = element.y != 0.0f ? element.y : StatusDisplayDefaults::Lives::Y;
        component.icon_size = element.icon_size != 0.0f ? element.icon_size : StatusDisplayDefaults::Lives::ICON_SIZE;
        component.icon_spacing = element.icon_spacing != 0.0f ? element.icon_spacing : StatusDisplayDefaults::Lives::ICON_SPACING;
        return component;
    }
};

struct ScoreDisplayComponent {
    static constexpr auto name = "ScoreDisplayComponent";

    int current_score = 0;
    int digit_count = StatusDisplayDefaults::Score::DIGIT_COUNT;
    float x = StatusDisplayDefaults::Score::X;
    float y = StatusDisplayDefaults::Score::Y;

    static ScoreDisplayComponent fromConfig(const UIConfig::Element& element) {
        ScoreDisplayComponent component;
        component.x = element.x != 0.0f ? element.x : StatusDisplayDefaults::Score::X;
        component.y = element.y != 0.0f ? element.y : StatusDisplayDefaults::Score::Y;
        component.digit_count = element.digit_count != 0 ? element.digit_count : StatusDisplayDefaults::Score::DIGIT_COUNT;
        return component;
    }
};

class StatusDisplayFactory {
public:
    static ChargeBarComponent createChargeBar(const UIConfig& config, const std::string& elementName = "ChargeBar") {
        auto it = config.elements.find(elementName);
        if (it != config.elements.end()) {
            return ChargeBarComponent::fromConfig(it->second);
        }
        return ChargeBarComponent{};
    }

    static LivesDisplayComponent createLivesDisplay(const UIConfig& config, const std::string& elementName = "LivesDisplay") {
        auto it = config.elements.find(elementName);
        if (it != config.elements.end()) {
            return LivesDisplayComponent::fromConfig(it->second);
        }
        return LivesDisplayComponent{};
    }

    static ScoreDisplayComponent createScoreDisplay(const UIConfig& config, const std::string& elementName = "ScoreDisplay") {
        auto it = config.elements.find(elementName);
        if (it != config.elements.end()) {
            return ScoreDisplayComponent::fromConfig(it->second);
        }
        return ScoreDisplayComponent{};
    }

    static StatusDisplayComponent createStatusDisplay() {
        StatusDisplayComponent component;
        component.is_initialized = true;
        component.player_entity = -1;
        return component;
    }
};
