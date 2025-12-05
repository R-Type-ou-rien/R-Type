cmake_minimum_required(VERSION 3.15)

# -----VCPKG CONFIG-----

set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")

if(WIN32)
    set(VCPKG_EXEC "${VCPKG_ROOT}/vcpkg.exe")
    set(VCPKG_BOOTSTRAP "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
else()
    set(VCPKG_EXEC "${VCPKG_ROOT}/vcpkg")
    set(VCPKG_BOOTSTRAP "${VCPKG_ROOT}/bootstrap-vcpkg.sh")
endif()

if(NOT EXISTS "${VCPKG_EXEC}")
    message(STATUS "Vcpkg non trouvé. Lancement du bootstrap automatique...")
    execute_process(COMMAND "${VCPKG_BOOTSTRAP}" WORKING_DIRECTORY "${VCPKG_ROOT}" RESULT_VARIABLE RES)
    if(NOT RES EQUAL 0)
        message(FATAL_ERROR "Erreur lors du bootstrap de vcpkg.")
    endif()
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain" FORCE)

# -----END VCPKG CONFIG-----

project(R-Type VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3.0 COMPONENTS Graphics Window System Audio Network CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

find_package(OpenGL REQUIRED)

# add_subdirectory(ecs) # -> lance le cmake a la root du directory ecs

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(R-Type src/main.cpp)

target_link_libraries(R-Type PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
        SFML::Network
        asio::asio
        OpenGL::GL
)