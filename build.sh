#!/bin/bash
set -e

mkdir -p build
cd build

cmake ../ --log-level=DEBUG

cmake --build .
