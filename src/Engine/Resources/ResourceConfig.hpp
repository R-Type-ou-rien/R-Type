#pragma once

#if defined(SERVER_BUILD)

template <typename ResourceType>
class ServerResourceManager;
template <typename T>
using ResourceManager = ServerResourceManager<T>;

struct TextureData {
    std::string pathname;
    TextureData(std::string path) { this->pathname = path; }
};

using TextureAsset = TextureData;
using SoundAsset = TextureData; // On server, sound is just a path/data too

#elif defined(CLIENT_BUILD)

template <typename ResourceType>
class ClientResourceManager;
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio.hpp>

template <typename T>
using ResourceManager = ClientResourceManager<T>;

using TextureAsset = sf::Texture;
using SoundAsset = sf::SoundBuffer;

#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif