#! /bin/bash
g++ ManipConvertor.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -o Convertor -g -Wall -pthread -std=c++17
