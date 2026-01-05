#pragma once

#include <functional>
#include <string>
#include "ECS/ECS.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "Context.hpp"
#include "Environment/Environment.hpp"

#define SUCCESS 0
#define FAILURE -1
#define USER_FUNCTION_SIGNATURE void(Environment & env, InputManager & inputs)

template <class Derived>
class GameEngineBase {
   protected:
    ECS _ecs;
    InputManager input_manager;
    ResourceManager<TextureAsset> _texture_manager;
    std::function<USER_FUNCTION_SIGNATURE> _loop_function;
    std::function<USER_FUNCTION_SIGNATURE> _init_function;
    // client network class

   public:
    explicit GameEngineBase() {}
    ~GameEngineBase() = default;

    int init() { return static_cast<Derived*>(this)->init(); }

    int run() { return static_cast<Derived*>(this)->run(); }

    void setLoopFunction(std::function<USER_FUNCTION_SIGNATURE> user_function) {
        _loop_function = user_function;
        return;
    }

    void setInitFunction(std::function<USER_FUNCTION_SIGNATURE> user_function) {
        _init_function = user_function;
        return;
    }
};