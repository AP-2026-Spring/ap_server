#!/bin/bash
set -e

echo "Starting vcpkg setup..."
cd ~
if [ ! -d "vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git
fi
cd vcpkg
./bootstrap-vcpkg.sh

echo "Installing dependencies (this may take 10-20 minutes)..."
./vcpkg install uwebsockets nlohmann-json openssl zlib

echo "Building the project..."
cd /mnt/d/Univ/4-2/adv/ap_server
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
make

echo "Done! You can now run ./DetectionServer"
