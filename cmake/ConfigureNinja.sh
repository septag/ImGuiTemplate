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

if [[ "$(uname)" == "Linux" ]]; then
    extra_cmake_opts="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang"
    echo Make sure you have installed these packages \(debian/ubuntu\):
    echo  pkg-config
    echo  libfreetype-dev
    echo  libglfw3-dev
    echo  uuid-dev
    echo  libgtk-3-dev
    echo  libc++abi-dev
else    
    extra_cmake_opts=""
fi

cmake ../../cmake -GNinja $extra_cmake_opts -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DAPPNAME="$@"

popd > /dev/null
popd > /dev/null
popd > /dev/null
