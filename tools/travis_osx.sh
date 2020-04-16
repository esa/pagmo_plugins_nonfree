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

# Install Pybind11 (making sure its the same used in our pipeline)
export PPNF_BUILD_DIR=`pwd`
git clone https://github.com/pybind/pybind11.git
cd pybind11
git checkout 4f72ef846fe8453596230ac285eeaa0ce3278bb4
mkdir build
cd build
pwd
cmake \
    -DPYBIND11_TEST=NO \
    -DCMAKE_INSTALL_PREFIX=$PPNF_BUILD_DIR \
    -DCMAKE_PREFIX_PATH=$PPNF_BUILD_DIR \
    ..
make install
cd ../..

# Build ppnf c++
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
        -Dpybind11_DIR=$PPNF_BUILD_DIR/share/cmake/pybind11 \
        ..

    make install VERBOSE=1;

    # Move out of the build dir.
    cd ../tools
    python -c "import pygmo_plugins_nonfree; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()";
fi

set +e

set +x
