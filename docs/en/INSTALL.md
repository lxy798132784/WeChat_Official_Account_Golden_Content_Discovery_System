# Installation Guide

## Linux from release tarball

```bash
tar -xzf premium-content-radar-*-linux-*.tar.gz
cd premium-content-radar-*-linux-*
./bin/premium-content-radar
```

If Qt plugin discovery fails, run from the extracted package root and keep the bundled `plugins/` directory next to the binary.

## Linux from source

Dependencies on Ubuntu or Debian-like systems:

```bash
sudo apt-get update
sudo apt-get install -y cmake g++ qt6-base-dev qt6-tools-dev libqt6sql6-sqlite libgl1-mesa-dev
```

Build and test:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/premium-content-radar --bridge-smoke
./build/premium-content-radar
```

## Windows from source

Install:

- Qt 6 for MSVC or MinGW.
- CMake.
- A matching C++ compiler.
- PowerShell 7 or Windows PowerShell.

Then run:

```powershell
$env:VERSION="1.0.1"
.\scripts\package-windows.ps1
```

## Docker build

```bash
docker buildx build --platform linux/amd64,linux/arm64 -t premium-content-radar:latest .
```

The Dockerfile is intended for build and remote desktop experimentation. Normal production use is the native desktop package.

## Uninstall

Delete the extracted package directory. Optional user data locations:

- SQLite database: configured in the WeChat settings tab.
- Runtime config: the platform app config directory for `premium-content-radar/settings.json`.
