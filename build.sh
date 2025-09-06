#!/bin/bash
set -euo pipefail

PRESET=${1:-nvidia-target}
BUILD_DIR=build/$PRESET

echo "Using preset: $PRESET, Build directory: $BUILD_DIR"

echo "#######################"
echo "###### CONFIGURE ######"
echo "#######################"
cmake --preset "$PRESET" -DPN_BUILD_TESTS=ON

echo "#######################"
echo "######## BUILD ########"
echo "#######################"
cmake --build --preset "$PRESET" -- -j3

echo "#######################"
echo "######## TEST #########"
echo "#######################"
#ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "#######################"
echo "####### INSTALL #######"
echo "#######################"
#cmake --install "$BUILD_DIR"
