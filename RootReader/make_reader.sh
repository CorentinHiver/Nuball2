#! /bin/bash
# g++ ManipReader.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o Reader -O2 -Wall -pthread -std=c++17
g++ ManipReader.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o Reader -g -Wall -pthread -std=c++17
