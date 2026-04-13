#!/usr/bin/env bash

if [ -z "$1" ]; then
  echo "Usage: $0 <search>"
  exit 1
fi

path=$(find . -type d -iname "*${1}*" | head -n 1)

echo $path 
