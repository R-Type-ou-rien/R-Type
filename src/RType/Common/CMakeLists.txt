cmake_minimum_required(VERSION 3.10)

project(RTypeCommon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ../../../
    ${CMAKE_CURRENT_SOURCE_DIR}/Components
)

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(RTypeCommon STATIC ${SOURCES})

target_link_libraries(RTypeCommon PRIVATE Engine)

