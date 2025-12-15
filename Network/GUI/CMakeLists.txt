cmake_minimum_required(VERSION 3.17)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
    set(VCPKG_ROOT "${REPO_ROOT}/vcpkg")
    
    if(EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Standalone build detected. Using vcpkg at ${VCPKG_ROOT}")
    else()
        message(WARNING "Vcpkg not found at ${VCPKG_ROOT}. Compilation might fail if dependencies are missing.")
    endif()
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(RTypeGUI LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(r-type-gui
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
)

target_link_libraries(r-type-gui PRIVATE Qt6::Widgets)
