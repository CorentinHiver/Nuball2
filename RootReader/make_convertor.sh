#! /bin/bash
g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o ManipConvertor -O2 -Wall -pthread -std=c++17
