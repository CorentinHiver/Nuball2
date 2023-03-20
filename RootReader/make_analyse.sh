#! /bin/bash
g++ Analyse.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o Analyse -O2 -Wall -pthread -std=c++17
