#include "AudioSystem.hpp"
#include "Components/AudioComponent.hpp"
#include <iostream>

#if defined(CLIENT_BUILD)
int AudioSystem::getAvailableSoundIndex() {
    for (size_t i = 0; i < _soundPool.size(); ++i) {
        if (_soundPool[i]->getStatus() == sf::Sound::Status::Stopped) {
            return static_cast<int>(i);
        }
    }

    if (_soundPool.size() < 64) {
        _soundPool.push_back(std::make_unique<sf::Sound>(_dummyBuffer));
        return static_cast<int>(_soundPool.size() - 1);
    }
    return 0;
}
#endif

void AudioSystem::update(Registry& registry, system_context context) {
#if defined(CLIENT_BUILD)
    auto& entities = registry.getEntities<AudioSourceComponent>();
    std::vector<int> entities_to_destroy;
    std::vector<int> components_to_remove;

    for (auto entity : entities) {
        auto& audio = registry.getComponent<AudioSourceComponent>(entity);

        if (audio.stop_requested) {
            if (audio.assigned_sound_index >= 0 && audio.assigned_sound_index < static_cast<int>(_soundPool.size())) {
                _soundPool[audio.assigned_sound_index]->stop();
            } else if (audio.assigned_sound_index == -2) {
                _bgMusic.stop();
            }
            components_to_remove.push_back(entity);
            continue;
        }

        if (audio.assigned_sound_index == -1) {
            if (audio.play_on_start) {
                if (context.sound_manager.is_loaded(audio.sound_name)) {
                    auto handleOpt = context.sound_manager.get_handle(audio.sound_name);
                    if (handleOpt) {
                        auto bufferOpt = context.sound_manager.get_resource(*handleOpt);
                        if (bufferOpt) {
                            int index = getAvailableSoundIndex();
                            audio.assigned_sound_index = index;

                            sf::Sound& sound = *_soundPool[index];

                            sound = sf::Sound(bufferOpt->get());
                            sound.setLooping(audio.loop);
                            sound.play();
                        }
                    }
                } else if (context.music_manager.is_loaded(audio.sound_name)) {
                    auto handleOpt = context.music_manager.get_handle(audio.sound_name);
                    if (handleOpt) {
                        auto pathOpt = context.music_manager.get_resource(*handleOpt);
                        if (pathOpt) {
                            if (_bgMusic.openFromFile(pathOpt->get())) {
                                _bgMusic.setLooping(audio.loop);
                                _bgMusic.play();
                                audio.assigned_sound_index = -2;
                            } else {
                                std::cerr << "[AUDIO] Failed to open music: " << pathOpt->get() << std::endl;
                            }
                        }
                    }
                }
            }
        }

        if (!audio.loop && audio.assigned_sound_index != -1) {
            if (audio.assigned_sound_index == -2) {
                if (_bgMusic.getStatus() == sf::SoundSource::Status::Stopped) {
                    if (audio.destroy_entity_on_finish) {
                        entities_to_destroy.push_back(entity);
                    } else {
                        components_to_remove.push_back(entity);
                    }
                }
            } else if (_soundPool[audio.assigned_sound_index]->getStatus() == sf::Sound::Status::Stopped) {
                if (!audio.next_sound_name.empty()) {
                    audio.sound_name = audio.next_sound_name;
                    audio.loop = audio.next_sound_loop;
                    audio.next_sound_name = "";
                    audio.assigned_sound_index = -1;
                } else {
                    if (audio.destroy_entity_on_finish) {
                        entities_to_destroy.push_back(entity);
                    } else {
                        components_to_remove.push_back(entity);
                    }
                }
            }
        }
    }

    for (auto entity : components_to_remove) {
        registry.removeComponent<AudioSourceComponent>(entity);
    }

    for (auto entity : entities_to_destroy) {
        registry.destroyEntity(entity);
    }
#endif
}
