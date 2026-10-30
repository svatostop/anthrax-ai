# !/bin/bash

[ -d build ] || mkdir build
cd build
# cmake -DCMAKE_BUILD_TYPE=DEBUG .. && cmake --build . -j
cmake -S .. -B . -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . 
