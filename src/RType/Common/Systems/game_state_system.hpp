/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game State System - Détecte le game over et synchronise client/serveur
*/

#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "Context.hpp"
#include "health.hpp"
#include "Components/StandardComponents.hpp"

/**
 * Système qui détecte la fin de partie (tous les joueurs morts ou boss vaincu)
 * et envoie les notifications réseau appropriées
 */
class GameStateSystem : public ISystem {
   public:
    void update(Registry& registry, system_context context) override;

   private:
    bool _gameOverSent = false;
    bool _victorySent = false;
};
