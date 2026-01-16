#!/bin/bash

if [ -d vcpkg ]; then
    git submodule update --init --recursive
fi

if [ "$1" == "clean" ]; then
    echo "--- Cleaning all builds ---"
    rm -rf build-client build-server build
    exit 0
fi

build_client() {
    echo "--- Building Client ---"
    cmake -B build-client -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SERVER=OFF
    cmake --build build-client -v
}

build_server() {
    echo "--- Building Server ---"
    cmake -B build-server -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SERVER=ON
    cmake --build build-server -v
}

if [ -z "$1" ]; then
    build_client
    echo ""
    build_server
elif [ "$1" == "client" ]; then
    build_client
elif [ "$1" == "server" ]; then
    build_server
else
    echo "Unknown argument: $1"
    echo "Usage: $0 [client|server|clean]"
    exit 1
fi
