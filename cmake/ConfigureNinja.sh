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

cmake ../../cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 "$@"

popd > /dev/null
popd > /dev/null
popd > /dev/null