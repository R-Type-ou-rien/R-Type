/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Level Transition System Implementation
*/

#include "level_transition.hpp"
#include <iostream>
#include "../Components/scripted_spawn.hpp"
#include "../Components/spawn.hpp"

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
            if (transition.transition_time > 2.0f) {
                transition.state = LevelTransitionComponent::TransitionState::FADING_OUT;
                transition.transition_time = 0.0f;
            }
            break;
            
        case LevelTransitionComponent::TransitionState::FADING_OUT:
            transition.fade_alpha = std::min(1.0f, transition.transition_time / transition.fade_duration);
            createFadeEffect(registry, transition.fade_alpha);
            
            if (transition.transition_time >= transition.fade_duration) {
                transition.state = LevelTransitionComponent::TransitionState::SHOW_LEVEL_COMPLETE;
                transition.transition_time = 0.0f;
                createLevelCompleteText(registry);
            }
            break;
            
        case LevelTransitionComponent::TransitionState::SHOW_LEVEL_COMPLETE:
            if (transition.transition_time >= transition.display_duration) {
                transition.state = LevelTransitionComponent::TransitionState::FADING_IN;
                transition.transition_time = 0.0f;
                
                std::cout << "[LevelTransition] Loading next level: " 
                          << transition.next_level_name << std::endl;

                // Réinitialiser les spawners avec le nouveau script
                auto& spawners = registry.getEntities<ScriptedSpawnComponent>();
                for (auto spawner : spawners) {
                    auto& script = registry.getComponent<ScriptedSpawnComponent>(spawner);
                    
                    // Charger le nouveau fichier de niveau
                    script.script_path = transition.next_level_name;
                    script.spawn_events.clear();
                    script.next_event_index = 0;
                    script.level_time = 0.0f;
                    script.all_events_completed = false;

                    // Réinitialiser l'état du boss et du timer global
                    if (registry.hasComponent<EnemySpawnComponent>(spawner)) {
                        auto& enemySpawn = registry.getComponent<EnemySpawnComponent>(spawner);
                        enemySpawn.boss_spawned = false;
                        enemySpawn.boss_arrived = false;
                        enemySpawn.total_time = 0.0f;
                        enemySpawn.is_active = true;
                    }
                }
            }
            break;
            
        case LevelTransitionComponent::TransitionState::FADING_IN:
            transition.fade_alpha = 1.0f - std::min(1.0f, transition.transition_time / transition.fade_duration);
            createFadeEffect(registry, transition.fade_alpha);
            
            if (transition.transition_time >= transition.fade_duration) {
                transition.state = LevelTransitionComponent::TransitionState::COMPLETE;
                registry.destroyEntity(entity);
            }
            break;
            
        case LevelTransitionComponent::TransitionState::COMPLETE:
            break;
    }
}

void LevelTransitionSystem::createFadeEffect(Registry& registry, float alpha) {    
    auto& fadeEntities = registry.getEntities<TagComponent>();
    bool fadeExists = false;
    
    for (auto entity : fadeEntities) {
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "FADE_OVERLAY") {
                fadeExists = true;
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
        
        TextComponent fadeText;
        fadeText.text = "";
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
