#!/bin/bash
set -e

export CMAKE_PREFIX_PATH=/opt/vision-common/lib/cmake

mkdir -p build
cd build

cmake ../ -DCMAKE_FIND_DEBUG_MODE=ON

cmake --build .

cmake --install .
