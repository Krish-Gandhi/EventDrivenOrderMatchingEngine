#!/bin/bash

BUILD_DIR="build/"
BINDING_DEST="../../python/"
BINDING_TARGET="engine_py.cpython-312-x86_64-linux-gnu.so"

cd $BUILD_DIR

echo "Running cmake in build..."
cmake ..

echo "Running make in build..."
make

echo "Moving .so file to ../python/ ..."
mv $BINDING_TARGET $BINDING_DEST

echo "Build complete."