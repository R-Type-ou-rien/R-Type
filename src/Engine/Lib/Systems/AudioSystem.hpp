#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "Context.hpp"
#include <vector>
#include <memory>

#if defined(CLIENT_BUILD)
#include <SFML/Audio.hpp>
#endif

class AudioSystem : public ISystem {
   public:
    AudioSystem() = default;
    ~AudioSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
#if defined(CLIENT_BUILD)
    std::vector<std::unique_ptr<sf::Sound>> _soundPool;
    sf::SoundBuffer _dummyBuffer;
    int getAvailableSoundIndex();
#endif
};
