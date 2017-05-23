#!/usr/bin/env bash

# Echo each command
set -x

if [[ "${PAGMO_BUILD}" != manylinux* ]]; then
    export PATH="$deps_dir/bin:$PATH"
fi

if [[ "${PAGMO_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_SNOPT7_BUILD_PYTHON=yes -DCMAKE_CXX_FLAGS="-fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_SNOPT7_BUILD_PYTHON=yes -DCMAKE_CXX_FLAGS="-fsanitize=address -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
fi

set +e
set +x