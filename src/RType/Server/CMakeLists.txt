cmake_minimum_required(VERSION 3.10)

project(RTypeServer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ../../../
)

file(GLOB_RECURSE SOURCES "*.cpp")

add_executable(r-type-server ${SOURCES})

target_link_libraries(r-type-server PRIVATE Engine RTypeCommon)
