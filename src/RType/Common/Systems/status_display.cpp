/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Status Display System implementation for R-Type style UI
*/

#include "status_display.hpp"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <string>

#include "Components/StandardComponents.hpp"
#include "../Components/charged_shot.hpp"
#include "../Components/status_display_components.hpp"
#include "health.hpp"
#include "score.hpp"

void StatusDisplaySystem::update(Registry& registry, system_context context) {
#if defined(CLIENT_BUILD)
    auto& statusEntities = registry.getEntities<StatusDisplayComponent>();
    if (!statusEntities.empty()) {
        auto& status = registry.getComponent<StatusDisplayComponent>(statusEntities[0]);
        if (status.getPlayerEntity() == -1) {
            auto& teams = registry.getEntities<TeamComponent>();
            for (auto entity : teams) {
                auto& team = registry.getConstComponent<TeamComponent>(entity);
                if (team.team == TeamComponent::ALLY) {
                    if (registry.hasComponent<TagComponent>(entity)) {
                        auto& tags = registry.getConstComponent<TagComponent>(entity);
                        for (const auto& tag : tags.tags) {
                            if (tag == "PLAYER") {
                                // Validate ownership
                                if (registry.hasComponent<NetworkIdentity>(entity)) {
                                    auto& netId = registry.getConstComponent<NetworkIdentity>(entity);
                                    if (netId.ownerId == context.player_id) {
                                        status.player_entity = entity;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                if (status.getPlayerEntity() != -1)
                    break;
            }
        }
    }

    drawChargeBar(registry, context);
    drawLives(registry, context);
    drawScore(registry, context);
#endif
}

void StatusDisplaySystem::drawChargeBar(Registry& registry, system_context& context) {
#if defined(CLIENT_BUILD)
    auto& chargeBarEntities = registry.getEntities<ChargeBarComponent>();
    if (chargeBarEntities.empty()) {
        return;
    }

    auto& statusEntities = registry.getEntities<StatusDisplayComponent>();
    if (statusEntities.empty()) {
        return;
    }

    auto& status = registry.getConstComponent<StatusDisplayComponent>(statusEntities[0]);
    if (status.getPlayerEntity() == -1) {
        return;
    }

    float charge_ratio = 0.0f;
    if (registry.hasComponent<ChargedShotComponent>(status.getPlayerEntity())) {
        auto& charged = registry.getConstComponent<ChargedShotComponent>(status.getPlayerEntity());
        if (charged.is_charging) {
            charge_ratio = charged.charge_time / charged.max_charge_time;
        }
    }

    auto& chargeBar = registry.getConstComponent<ChargeBarComponent>(chargeBarEntities[0]);

    float bar_x = chargeBar.getX();
    float bar_y = chargeBar.getY();
    float bar_width = chargeBar.getBarWidth();
    float bar_height = chargeBar.getBarHeight();

    sf::RectangleShape background({bar_width + 4, bar_height + 4});
    background.setPosition({bar_x - 2, bar_y - 2});
    background.setFillColor(sf::Color(40, 40, 40));
    background.setOutlineColor(sf::Color(100, 100, 100));
    background.setOutlineThickness(2);
    context.window.draw(background);

    sf::RectangleShape emptyBar({bar_width, bar_height});
    emptyBar.setPosition({bar_x, bar_y});
    emptyBar.setFillColor(sf::Color(60, 60, 80));
    context.window.draw(emptyBar);

    if (charge_ratio > 0.0f) {
        float fill_width = bar_width * charge_ratio;
        sf::RectangleShape fillBar({fill_width, bar_height});
        fillBar.setPosition({bar_x, bar_y});

        sf::Color barColor;
        if (charge_ratio < 0.5f) {
            barColor = sf::Color(100, 150, 255);
        } else if (charge_ratio < 0.8f) {
            barColor = sf::Color(255, 200, 50);
        } else {
            barColor = sf::Color(255, 100, 50);
        }
        fillBar.setFillColor(barColor);
        context.window.draw(fillBar);
    }

    if (!_fontLoaded) {
        if (!_font.openFromFile("src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf")) {
            std::cerr << "StatusDisplaySystem: Failed to load font" << std::endl;
            return;
        }
        _fontLoaded = true;
    }

    sf::Text label(_font);
    label.setString("BEAM");
    label.setCharacterSize(16);
    label.setFillColor(sf::Color::White);
    label.setPosition({bar_x + bar_width / 2 - 25, bar_y - 25});
    context.window.draw(label);
#endif
}

void StatusDisplaySystem::drawLives(Registry& registry, system_context& context) {
#if defined(CLIENT_BUILD)
    auto& livesEntities = registry.getEntities<LivesDisplayComponent>();
    if (livesEntities.empty()) {
        return;
    }

    auto& statusEntities = registry.getEntities<StatusDisplayComponent>();
    if (statusEntities.empty()) {
        return;
    }

    auto& status = registry.getConstComponent<StatusDisplayComponent>(statusEntities[0]);
    if (status.getPlayerEntity() == -1) {
        return;
    }

    int current_lives = 0;
    if (registry.hasComponent<HealthComponent>(status.getPlayerEntity())) {
        auto& health = registry.getConstComponent<HealthComponent>(status.getPlayerEntity());
        current_lives = health.current_hp;
    }

    auto& livesDisplay = registry.getConstComponent<LivesDisplayComponent>(livesEntities[0]);

    for (int i = 0; i < current_lives && i < 5; i++) {
        sf::RectangleShape lifeIcon({livesDisplay.getIconSize(), livesDisplay.getIconSize() * 0.6f});
        lifeIcon.setPosition({livesDisplay.getX() + i * livesDisplay.getIconSpacing(), livesDisplay.getY()});
        lifeIcon.setFillColor(sf::Color(50, 150, 255));
        lifeIcon.setOutlineColor(sf::Color::White);
        lifeIcon.setOutlineThickness(1);
        context.window.draw(lifeIcon);
    }
#endif
}

void StatusDisplaySystem::drawScore(Registry& registry, system_context& context) {
#if defined(CLIENT_BUILD)
    auto& scoreDisplayEntities = registry.getEntities<ScoreDisplayComponent>();
    if (scoreDisplayEntities.empty()) {
        return;
    }

    auto& scoreDisplay = registry.getConstComponent<ScoreDisplayComponent>(scoreDisplayEntities[0]);

    int score = 0;
    bool resolved = false;

    auto& statusEntities = registry.getEntities<StatusDisplayComponent>();
    if (!statusEntities.empty()) {
        const auto& status = registry.getConstComponent<StatusDisplayComponent>(statusEntities[0]);
        if (status.getPlayerEntity() != -1 && registry.hasComponent<ScoreComponent>(status.getPlayerEntity())) {
            const auto& scoreComp = registry.getConstComponent<ScoreComponent>(status.getPlayerEntity());
            score = scoreComp.current_score;
            resolved = true;
        }
    }

    if (!resolved) {
        score = ScoreSystem::getScore(registry);
    }

    std::ostringstream oss;
    oss << std::setw(scoreDisplay.getDigitCount()) << std::setfill('0') << score;
    std::string scoreStr = oss.str();

    if (!_fontLoaded) {
        if (!_font.openFromFile("src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf")) {
            std::cerr << "StatusDisplaySystem: Failed to load font" << std::endl;
            return;
        }
        _fontLoaded = true;
    }

    sf::Text scoreLabel(_font);
    scoreLabel.setString("SCORE");
    scoreLabel.setCharacterSize(16);
    scoreLabel.setFillColor(sf::Color::White);
    scoreLabel.setPosition({scoreDisplay.getX(), scoreDisplay.getY() - 25});
    context.window.draw(scoreLabel);

    sf::Text scoreText(_font);
    scoreText.setString(scoreStr);
    scoreText.setCharacterSize(28);
    scoreText.setFillColor(sf::Color::Yellow);
    scoreText.setPosition({scoreDisplay.getX(), scoreDisplay.getY()});
    context.window.draw(scoreText);
#endif
}
