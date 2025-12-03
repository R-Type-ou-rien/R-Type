#!/bin/bash

if [ "$1" == "clean" ] && [ -d build ]; then
    cmake --build build --target clean
    rm -rf build
    exit 0
fi

if [ -d build ]; then
    rm -rf build
fi

cmake -B build

cmake build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug


cmake --build build
