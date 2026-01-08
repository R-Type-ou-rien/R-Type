#pragma once

#include <string>

#if defined(SERVER_BUILD)

template <typename ResourceType>
class ServerResourceManager;
template <typename T>
using ResourceManager = ServerResourceManager<T>;

struct TextureData {
    std::string pathname;
    TextureData() = default;
    TextureData(std::string path) { this->pathname = path; }
};

using TextureAsset = TextureData;
using SoundAsset = TextureData;
using MusicAsset = TextureData;

#elif defined(CLIENT_BUILD)

template <typename ResourceType>
class ClientResourceManager;
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio.hpp>

template <typename T>
using ResourceManager = ClientResourceManager<T>;

using TextureAsset = sf::Texture;
using SoundAsset = sf::SoundBuffer;
using MusicAsset = std::string;

#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif