#!/bin/zsh

cmake -B./build -S. -GNinja && cmake --build ./build

