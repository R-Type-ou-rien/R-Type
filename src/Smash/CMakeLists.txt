cmake_minimum_required(VERSION 3.10)

project(Smash)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Engine
    ${CMAKE_SOURCE_DIR}/src/Engine/Core
    ${CMAKE_SOURCE_DIR}/src/Engine/Core/ECS
    ${CMAKE_SOURCE_DIR}/src/Engine/Lib
    ${CMAKE_SOURCE_DIR}/src/Engine/Lib/Components
    ${CMAKE_SOURCE_DIR}/src/Engine/Graphics
    ${CMAKE_SOURCE_DIR}/src/Engine/Inputs
)

file(GLOB_RECURSE SOURCES "*.cpp")

set(RTYPE_BEHAVIOR_SOURCE "${CMAKE_SOURCE_DIR}/src/RType/Common/Systems/behavior.cpp")

add_executable(smash ${SOURCES} ${RTYPE_BEHAVIOR_SOURCE})

target_link_libraries(smash PRIVATE Engine)