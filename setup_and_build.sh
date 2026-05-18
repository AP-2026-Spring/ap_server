#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

echo "Starting vcpkg setup..."
cd ~
if [ ! -d "vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git
fi
cd vcpkg
./bootstrap-vcpkg.sh

echo "Installing dependencies (this may take 10-20 minutes)..."
./vcpkg install uwebsockets nlohmann-json openssl zlib sqlite3

echo "Building the project..."
cd "$PROJECT_DIR"
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
make

echo "Done! You can now run ./DetectionServer"
