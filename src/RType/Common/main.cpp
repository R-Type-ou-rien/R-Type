#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <ostream>

#include "GameEngineConfig.hpp"
#include "Components/StandardComponents.hpp"
#include "Lib/GameManager/GameManager.hpp"

/**
 Comment:
 WARNING: BIG ISSUE ECS -> le client ET le serveur créent les entités de la game logique
*/

int main() {
    GameEngine engine;
    GameManager gm;

    engine.setInitFunction([&gm](ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures)
        { gm.init(ecs, inputs, textures); }
    );

    engine.setLoopFunction([&gm](ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures)
        { gm.update(ecs, inputs, textures); }
    );
    engine.run();
    return 0;
}
