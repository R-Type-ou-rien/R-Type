#!/bin/bash
set -e

# Se positionner à la racine du projet (parent de launch_project)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

echo "=== Working directory: $PROJECT_ROOT ==="

# Initialiser le submodule vcpkg si nécessaire
if [ ! -d "vcpkg/.git" ]; then
    echo "--- Initializing vcpkg submodule ---"
    git submodule update --init --recursive
fi

# Bootstrap vcpkg si nécessaire
if [ ! -f "vcpkg/vcpkg" ]; then
    echo "--- Bootstrapping vcpkg ---"
    ./vcpkg/bootstrap-vcpkg.sh
fi

# Installer les dépendances vcpkg (mode manifest - lit vcpkg.json)
echo "--- Installing vcpkg dependencies (this may take a while) ---"
export VCPKG_DEFAULT_TRIPLET=x64-linux
./vcpkg/vcpkg install

if [ "$1" == "clean" ]; then
    echo "--- Cleaning all builds ---"
    rm -rf build-client build-server build
    exit 0
fi

build_client() {
    echo "--- Building Client ---"
    cmake -B build-client -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SERVER=OFF \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET=x64-linux \
        -DCMAKE_PREFIX_PATH="$PROJECT_ROOT/vcpkg_installed/x64-linux" \
        -DVCPKG_INSTALLED_DIR="$PROJECT_ROOT/vcpkg_installed"
    cmake --build build-client -v
}

build_server() {
    echo "--- Building Server ---"
    cmake -B build-server -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SERVER=ON \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET=x64-linux \
        -DCMAKE_PREFIX_PATH="$PROJECT_ROOT/vcpkg_installed/x64-linux" \
        -DVCPKG_INSTALLED_DIR="$PROJECT_ROOT/vcpkg_installed"
    cmake --build build-server -v
}

build_smash() {
    echo "--- Building Smash ---"
    cmake -B build-smash -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SERVER=OFF -DBUILD_SMASH=ON \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET=x64-linux \
        -DCMAKE_PREFIX_PATH="$PROJECT_ROOT/vcpkg_installed/x64-linux" \
        -DVCPKG_INSTALLED_DIR="$PROJECT_ROOT/vcpkg_installed"
    cmake --build build-smash -v
}

if [ -z "$1" ]; then
    build_client
    echo ""
    build_server
elif [ "$1" == "client" ]; then
    build_client
elif [ "$1" == "server" ]; then
    build_server
elif [ "$1" == "smash" ]; then
    build_smash
else
    echo "Unknown argument: $1"
    echo "Usage: $0 [client|server|clean]"
    exit 1
fi
