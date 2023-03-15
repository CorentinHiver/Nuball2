#! /bin/bash
g++ rootReader.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o Reader -O2 -Wall -pthread -std=c++17
