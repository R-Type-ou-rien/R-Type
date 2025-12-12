cmake_minimum_required(VERSION 3.10)

project(Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML REQUIRED COMPONENTS Graphics Window System)


file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Inputs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Actors/*.cpp"
)

add_library(Engine STATIC ${SOURCES})

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/Registry
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS/SystemManager
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/Inputs
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib/Systems
    "${CMAKE_CURRENT_SOURCE_DIR}/Lib/Actors/"
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources
)

target_link_libraries(Engine PUBLIC SFML::Graphics SFML::Window SFML::System)
