if (Test-Path .\vcpkg) {
    git submodule update --init --recursive
}

if ($args[0] -eq "clean" -and (Test-Path .\vcpkg)) {
    cmake --build build --target clean
    if (Test-Path .\build) {
        Remove-Item -Recurse -Force .\build
    }
    if (Test-Path .\Debug) {
        Remove-Item -Recurse -Force .\Debug
    }
    if (Test-Path .\Release) {
        Remove-Item -Recurse -Force .\Release
    }
    if (Test-Path .\r-type-client.exe) {
        Remove-Item -Force .\r-type-client.exe
    }
    if (Test-Path .\r-type-server.exe) {
        Remove-Item -Force .\r-type-server.exe
    }
    exit
}

if (-not (Test-Path .\build)) {
    cmake -B build -DCMAKE_BUILD_TYPE=Debug
}

cmake --build build --config Release -v

if (Test-Path .\Release\r-type-client.exe) {
    Copy-Item .\Release\r-type-client.exe .
}

if (Test-Path .\Debug\r-type-client.exe) {
    Copy-Item .\Debug\r-type-client.exe .
}