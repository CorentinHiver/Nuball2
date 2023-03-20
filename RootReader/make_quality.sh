#! /bin/bash
g++ Quality.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o Quality -O2 -Wall -pthread -std=c++17
