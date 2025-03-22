#!/bin/sh

rm -rf ./build_target ./target

mkdir -p ./build_target
g++ -std=c++17 -o ./build_target/build build.cpp && ./build_target/build
