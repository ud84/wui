#!/bin/sh

mkdir build
cd build
cmake ..
make -j 4
echo "WUI Ready!"
