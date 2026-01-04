# !/bin/bash

[ -d build ] || mkdir build
cd build
# cmake -DCMAKE_BUILD_TYPE=DEBUG .. && cmake --build . -j
cmake -DCMAKE_BUILD_TYPE=DEBUG .. && cmake --build . 
