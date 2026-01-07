cmake_minimum_required(VERSION 3.10)

project(Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/*.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Actors/*.cpp"
)

add_library(Engine STATIC ${CORE_SOURCES})

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/Registry
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/SystemManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Actors"
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources
)

target_link_libraries(Engine PUBLIC SFML::Graphics SFML::Window SFML::System NetworkLib)

find_package(SFML REQUIRED COMPONENTS Window Graphics)
target_link_libraries(Engine PUBLIC SFML::Window SFML::Graphics)

file(GLOB_RECURSE COMMON_SYSTEM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/PatternSystem/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/CollisionSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/PhysicsSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/ScrollSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/SpawnSystem.cpp"
)

if (BUILD_SERVER)
    target_compile_definitions(Engine PUBLIC SERVER_BUILD)

    file(GLOB_RECURSE SERVER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/ComponentSenderSystem/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/ActionScriptSystem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/ResourceSystem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/ServerGameEngine.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/ActionRegistry.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/InputState.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/InputAction.cpp"
    )
    target_sources(Engine PRIVATE ${COMMON_SYSTEM_SOURCES} ${SERVER_SOURCES})

else()
    target_compile_definitions(Engine PUBLIC CLIENT_BUILD)
    find_package(SFML REQUIRED COMPONENTS System Audio)
    target_link_libraries(Engine PUBLIC SFML::System SFML::Audio)

    file(GLOB_RECURSE CLIENT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/ActionScriptSystem.cpp" # comment: a retirer
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/BackgroundSystem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/RenderSystem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/InputSystem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/AudioSystem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/ClientGameEngine.cpp"
    )
    target_sources(Engine PRIVATE ${COMMON_SYSTEM_SOURCES} ${CLIENT_SOURCES})

endif()
