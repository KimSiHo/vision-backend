#!/bin/bash
set -euo pipefail

PRESET=${1:-nvidia-target}
BUILD_DIR=build/$PRESET

echo "Using preset: $PRESET, Build directory: $BUILD_DIR"

if [ ! -d "$BUILD_DIR" ]; then
  echo "Error: Build directory not found. Please run ./build.sh first."
  exit 1
fi

echo "#######################"
echo "######## TEST #########"
echo "#######################"
ctest --test-dir "$BUILD_DIR" --output-on-failure
