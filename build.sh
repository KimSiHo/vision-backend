#!/bin/bash
set -e

export CMAKE_PREFIX_PATH=/opt/vision-common/lib/cmake

mkdir -p build
cd build

cmake ../ --log-level=DEBUG

cmake --build .
