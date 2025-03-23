#!/bin/sh

rm -rf ./target
g++ -std=c++17 -o build build.cpp && ./build
