#!/usr/bin/env bash

mkdir build
cd build
cc -o calc ../main.c -lm
cd ..
if command -v rlwrap >/dev/null 2>&1; then
  rlwrap ./build/calc
else
  echo "rlwrap is NOT installed"

  ./build/calc

  ./build/calc
fi
