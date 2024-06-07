set windows-shell := ["pwsh", "-NoLogo", "-NoProfileLoadTime", "-Command"]

build:
    cmake --build build-debug
run: build
    ./build-debug/shapezx