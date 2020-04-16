#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

wget https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -O miniconda.sh;
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge --force

conda_pkgs="boost-cpp cmake pagmo-devel cxx-compiler"

if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == *Python37* ]]; then
    build_cpp_tests="no"
    conda_pkgs="$conda_pkgs python=3.7 pygmo pybind11"
else 
    build_cpp_tests="yes"
fi

conda create -q -p $deps_dir -y $conda_pkgs
source activate $deps_dir

export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
export PATH="$deps_dir/bin:$PATH"

export CXX=clang++
export CC=clang

cmake \
    -DCMAKE_INSTALL_PREFIX=$deps_dir \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DBoost_NO_BOOST_CMAKE=ON \
    -DCMAKE_BUILD_TYPE=${PAGMO_PLUGINS_NONFREE_BUILD_TYPE} \
    -DPPNF_BUILD_TESTS=$build_cpp_tests \
    ..

make -j2 VERBOSE=1;

if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != *Python* ]]; then
    # We run the cpp tests
    ctest -j4 -V
else
    # Compile pygmo_plugins_nonfree.
    cmake \
        -DCMAKE_INSTALL_PREFIX=$deps_dir \
        -DCMAKE_PREFIX_PATH=$deps_dir \
        -DBoost_NO_BOOST_CMAKE=ON \
        -DCMAKE_BUILD_TYPE=${PAGMO_PLUGINS_NONFREE_BUILD_TYPE} \
        -DPPNF_BUILD_TESTS=no \
        -DPPNF_BUILD_CPP=no \
        -DPPNF_BUILD_PYTHON=yes \
        ..

    make install VERBOSE=1;

    # Move out of the build dir.
    cd ../tools
    python -c "from dcgpy import test; test.run_test_suite(); import pygmo; pygmo.mp_island.shutdown_pool(); pygmo.mp_bfe.shutdown_pool()";
fi

set +e

set +x
