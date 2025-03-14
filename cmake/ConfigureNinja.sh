#!/bin/bash

pushd "$(dirname "$0")/../" > /dev/null

if [ ! -d .build ]; then
    mkdir .build
fi

pushd .build > /dev/null

if [ ! -d Ninja ]; then
    mkdir Ninja
fi

pushd Ninja > /dev/null

cmake ../../cmake -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DAPPNAME="$@"

popd > /dev/null
popd > /dev/null
popd > /dev/null
