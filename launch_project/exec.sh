#!/bin/bash

if [ -d vcpkg ]; then
    git submodule update --init --recursive
fi

if [ "$1" == "clean" ]; then
    if [ -d build ]; then
        cmake --build build --target clean
        rm -rf build
    fi
    exit 0
fi

if [ ! -d build ]; then
    cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
fi

cmake --build build -v
