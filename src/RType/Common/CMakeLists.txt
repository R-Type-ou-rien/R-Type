cmake_minimum_required(VERSION 3.10)

project(RTypeCommon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(RTypeCommon STATIC ${SOURCES})

target_include_directories(RTypeCommon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Components
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib
)

target_include_directories(RTypeCommon PRIVATE
    ../../../
)

target_link_libraries(RTypeCommon PRIVATE Engine)

