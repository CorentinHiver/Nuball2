#!/bin/bash

echo "Compiling libCo.pp"
g++ -O2 libCo.hpp -o Headers/libCo.hpp.precompiled
echo "Headers/libCo.hpp.precompiled written"
echo "Compiling libRoot.pp"
g++ libRoot.hpp -o Headers/libRoot.hpp.precompiled
echo "Headers/libRoot.hpp.precompiled written"