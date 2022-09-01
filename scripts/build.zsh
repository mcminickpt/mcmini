#!/bin/zsh

cmake -B./build -S. && cmake --build ./build -- -j5

