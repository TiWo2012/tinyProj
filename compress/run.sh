#!/usr/bin/env bash

mkdir -p build
cd build
cc -o compress ../main.c -lz
cd ..
./build/compress
