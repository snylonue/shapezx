set windows-shell := ["pwsh", "-NoLogo", "-NoProfileLoadTime", "-Command"]

build:
    cmake --build build
run: build
    ./build/shapezx