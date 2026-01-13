/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Level Transition System Implementation
*/

#include "level_transition.hpp"
#include <iostream>

void LevelTransitionSystem::update(Registry& registry, system_context context) {
    auto& transitions = registry.getEntities<LevelTransitionComponent>();
    
    for (auto entity : transitions) {
        if (!registry.hasComponent<LevelTransitionComponent>(entity))
            continue;
            
        auto& transition = registry.getComponent<LevelTransitionComponent>(entity);
        handleTransitionState(registry, entity, transition, context.dt);
    }
}

void LevelTransitionSystem::handleTransitionState(Registry& registry, Entity entity,
                                                   LevelTransitionComponent& transition,
                                                   float dt) {
    transition.transition_time += dt;
    
    switch (transition.state) {
        case LevelTransitionComponent::TransitionState::IDLE:
            // Attendre un moment avant de commencer
            if (transition.transition_time > 2.0f) {
                transition.state = LevelTransitionComponent::TransitionState::FADING_OUT;
                transition.transition_time = 0.0f;
            }
            break;
            
        case LevelTransitionComponent::TransitionState::FADING_OUT:
            // Fade out progressif
            transition.fade_alpha = std::min(1.0f, transition.transition_time / transition.fade_duration);
            createFadeEffect(registry, transition.fade_alpha);
            
            if (transition.transition_time >= transition.fade_duration) {
                transition.state = LevelTransitionComponent::TransitionState::SHOW_LEVEL_COMPLETE;
                transition.transition_time = 0.0f;
                createLevelCompleteText(registry);
            }
            break;
            
        case LevelTransitionComponent::TransitionState::SHOW_LEVEL_COMPLETE:
            // Afficher "LEVEL COMPLETE" pendant un moment
            if (transition.transition_time >= transition.display_duration) {
                transition.state = LevelTransitionComponent::TransitionState::FADING_IN;
                transition.transition_time = 0.0f;
                
                // TODO: Charger le niveau suivant ici
                std::cout << "[LevelTransition] Loading next level: " 
                          << transition.next_level_name << std::endl;
            }
            break;
            
        case LevelTransitionComponent::TransitionState::FADING_IN:
            // Fade in vers le nouveau niveau
            transition.fade_alpha = 1.0f - std::min(1.0f, transition.transition_time / transition.fade_duration);
            createFadeEffect(registry, transition.fade_alpha);
            
            if (transition.transition_time >= transition.fade_duration) {
                transition.state = LevelTransitionComponent::TransitionState::COMPLETE;
                // Supprimer l'entité de transition
                registry.destroyEntity(entity);
            }
            break;
            
        case LevelTransitionComponent::TransitionState::COMPLETE:
            // Transition terminée
            break;
    }
}

void LevelTransitionSystem::createFadeEffect(Registry& registry, float alpha) {
    // Créer un effet de fade avec un rectangle semi-transparent
    // Note: Cette implémentation est simplifiée - dans un système de rendu complet,
    // vous utiliseriez un sprite ou un rectangle dessiné directement
    
    auto& fadeEntities = registry.getEntities<TagComponent>();
    bool fadeExists = false;
    
    for (auto entity : fadeEntities) {
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "FADE_OVERLAY") {
                fadeExists = true;
                // L'overlay existe déjà, on pourrait mettre à jour son alpha
                // mais c'est géré par le système de rendu
                break;
            }
        }
        if (fadeExists) break;
    }
    
    if (!fadeExists && alpha > 0.0f) {
        Entity fadeEntity = registry.createEntity();
        
        TagComponent tags;
        tags.tags.push_back("FADE_OVERLAY");
        registry.addComponent<TagComponent>(fadeEntity, tags);
        
        // Créer un texte pour simuler un overlay (solution simplifiée)
        // Dans un vrai jeu, vous utiliseriez un composant graphique approprié
        TextComponent fadeText;
        fadeText.text = "";  // Vide, juste pour marquer l'entité
        fadeText.fontPath = "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf";
        fadeText.characterSize = 1;
        fadeText.color = sf::Color(0, 0, 0, static_cast<uint8_t>(alpha * 255));
        fadeText.x = 0;
        fadeText.y = 0;
        registry.addComponent<TextComponent>(fadeEntity, fadeText);
    }
}

void LevelTransitionSystem::createLevelCompleteText(Registry& registry) {
    Entity textEntity = registry.createEntity();
    
    TagComponent tags;
    tags.tags.push_back("LEVEL_COMPLETE_TEXT");
    registry.addComponent<TagComponent>(textEntity, tags);
    
    registry.addComponent<TextComponent>(
        textEntity,
        {"LEVEL COMPLETE!",
         "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
         64,
         sf::Color::Green,
         650,
         400});
    
    // Texte secondaire
    Entity subText = registry.createEntity();
    registry.addComponent<TextComponent>(
        subText,
        {"Proceeding to next level...",
         "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
         32,
         sf::Color::White,
         700,
         480});
}
