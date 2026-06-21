# !/bin/bash

[ -d build ] || mkdir build
cd build
# cmake -DCMAKE_BUILD_TYPE=DEBUG .. && cmake --build . -j
cmake -G "Ninja" -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . 
