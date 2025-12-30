cmake_minimum_required(VERSION 3.10)

project(Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SERVER "Build for server instead of client" OFF)

file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Actors/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/InputState.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/InputAction.cpp"
)

add_library(Engine STATIC ${CORE_SOURCES})

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/Registry
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/SystemManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Actors"
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs
)

if(BUILD_SERVER)
    target_compile_definitions(Engine PUBLIC SERVER_BUILD)
else()
    target_compile_definitions(Engine PUBLIC CLIENT_BUILD)

    find_package(SFML REQUIRED COMPONENTS Graphics Window System)

    file(GLOB_RECURSE CLIENT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/*.cpp"
    )
    target_sources(Engine PRIVATE ${CLIENT_SOURCES})

    target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
        ${CMAKE_CURRENT_SOURCE_DIR}/Inputs
    )

    target_link_libraries(Engine PUBLIC SFML::Graphics SFML::Window SFML::System)
endif()
