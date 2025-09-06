#!/bin/bash
set -euo pipefail

PRESET=${1:-nvidia-target}
BUILD_DIR=build/$PRESET

echo "Using preset: $PRESET, Build directory: $BUILD_DIR"

echo "#######################"
echo "###### CONFIGURE ######"
echo "#######################"
cmake --preset "$PRESET"

echo "#######################"
echo "######## BUILD ########"
echo "#######################"
cmake --build --preset "$PRESET" -- -j$(nproc)

echo "#######################"
echo "####### INSTALL #######"
echo "#######################"
#cmake --install "$BUILD_DIR"
