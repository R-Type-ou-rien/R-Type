if (Test-Path .\vcpkg) {
    git submodule update --init --recursive
}

if ($args[0] -eq "clean") {
    Write-Host "--- Cleaning all builds ---"
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue .\build-client, .\build-server, .\build, .\Debug, .\Release
    Remove-Item -Force -ErrorAction SilentlyContinue .\r-type-client.exe, .\r-type-server.exe
    exit
}

function Build-Client {
    Write-Host "--- Building Client ---"
    cmake -B build-client -DBUILD_SERVER=OFF
    cmake --build build-client --config Release -v
    if (Test-Path .\Release\r-type-client.exe) {
        Copy-Item .\Release\r-type-client.exe . -Force
    }
}

function Build-Server {
    Write-Host "--- Building Server ---"
    cmake -B build-server -DBUILD_SERVER=ON
    cmake --build build-server --config Release -v
    if (Test-Path .\Release\r-type-server.exe) {
        Copy-Item .\Release\r-type-server.exe . -Force
    }
}

if ($args.Count -eq 0) {
    Build-Client
    Build-Server
}
elseif ($args[0] -eq "client") {
    Build-Client
}
elseif ($args[0] -eq "server") {
    Build-Server
}
else {
    Write-Host "Unknown argument: $($args[0])"
    Write-Host "Usage: .\win_exec.ps1 [client|server|clean]"
    exit 1
}