#! /bin/bash
g++ FasterToRoot.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -pthread -std=c++17 -o FasterToRoot -O2 -Wall -Wextra
# g++ FasterToRoot.cpp  $(root-config --glibs --cflags --libs) $(pkg-config  --cflags --libs libfasterac) -pthread -std=c++17 -o FasterToRoot -g -Wall -Wextra
