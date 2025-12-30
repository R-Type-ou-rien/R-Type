#pragma once

#if defined(SERVER_BUILD)
    #include "ServerResourceManager.hpp"

    template<typename T>
    using ResourceManager = ServerResourceManager<T>;

    struct TextureData {
        std::string pathname;
    };

    using TextureAsset = TextureData;

#elif defined(CLIENT_BUILD)
    #include "ClientResourceManager.hpp"
    #include <SFML/Graphics/Texture.hpp>

    template<typename T>
    using ResourceManager = ClientResourceManager<T>;

    using TextureAsset = sf::Texture;

#else
    #error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif