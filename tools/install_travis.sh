#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# If needed we make sure conda stuff is found
if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != manylinux* ]]; then
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    export PATH="$deps_dir/bin:$PATH"
fi

if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "CoverageGCC5" ]]; then
    CXX=g++-5 CC=gcc-5 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="--coverage -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
    bash <(curl -s https://codecov.io/bash) -x gcov-5;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugGCC6" ]]; then
    CXX=g++-6 CC=gcc-6 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "ReleaseClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "OSXDebug" ]]; then
    CXX=clang++ CC=clang cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-g0 -O2" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "OSXRelease" ]]; then
    CXX=clang++ CC=clang cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == Python* ]]; then
    export CXX=g++-4.8
    export CC=gcc-4.8
    # Install pygmo_plugins_nonfree.
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_PYTHON=yes -DPAGMO_PLUGINS_NONFREE_HEADERS=no ../;
    make install VERBOSE=1;
    ipcluster start --daemonize=True;
    # Give some time for the cluster to start up. The cluster may be needed in tests of archipelagos with ipyparallel islands
    sleep 20;
    # Move out of the build dir.
    cd ../tools
    # Run the test suite
    python -c "import pygmo_plugins_nonfree as pg7; pg7.test.run_test_suite(1)";
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == OSXPython* ]]; then
    export CXX=clang++
    export CC=clang
    # Install pagmo first.
    cd ..;
    mkdir build_pagmo;
    cd build_pagmo;
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_WITH_EIGEN3=yes -DPAGMO_WITH_NLOPT=yes -DPAGMO_WITH_IPOPT=yes ../;
    make install VERBOSE=1;
    cd ../build;
    # Now pygmo.
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_PYGMO=yes -DPAGMO_PLUGINS_NONFREE_BUILD_PAGMO=no -DCMAKE_CXX_FLAGS="-g0 -O2" ../;
    make install VERBOSE=1;
    ipcluster start --daemonize=True;
    # Give some time for the cluster to start up.
    sleep 20;
    # Move out of the build dir.
    cd ../tools
    python -c "import pygmo; pygmo.test.run_test_suite(1)"

    # Additional serialization tests.
    python travis_additional_tests.py

    # AP examples.
    cd ../ap_examples/uda_basic;
    mkdir build;
    cd build;
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug ../;
    make install VERBOSE=1;
    cd ../../;
    python test1.py

    cd udp_basic;
    mkdir build;
    cd build;
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug ../;
    make install VERBOSE=1;
    cd ../../;
    python test2.py
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == manylinux* ]]; then
    cd ..;
    docker pull ${DOCKER_IMAGE};
    docker run --rm -e TWINE_PASSWORD -e PAGMO_PLUGINS_NONFREE_BUILD -e TRAVIS_TAG -v `pwd`:/pagmo2 $DOCKER_IMAGE bash /pagmo2/tools/install_docker.sh
fi

set +e
set +x