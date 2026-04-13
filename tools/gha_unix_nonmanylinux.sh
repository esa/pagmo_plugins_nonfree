#!/usr/bin/env bash

set -x
set -e

: "${PPNF_PYTHON_VERSION:?PPNF_PYTHON_VERSION must be set}"

# Ensure conda-forge is used consistently across all non-manylinux Unix jobs.
conda config --add channels conda-forge
conda config --set channel_priority strict
conda install -y -q \
    c-compiler cxx-compiler ninja "cmake>=3.20" \
    "python=${PPNF_PYTHON_VERSION}" \
    pagmo-devel "libboost-devel>=1.86" pybind11 pybind11-abi pygmo

deps_dir="${CONDA_PREFIX}"

# Build and install the C++ library.
rm -rf build_cpp
mkdir build_cpp
cd build_cpp
cmake ../ \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${deps_dir}" \
    -DCMAKE_PREFIX_PATH="${deps_dir}"
cmake --build . --target install --parallel 2
cd ..

# Build and install the Python extension.
rm -rf build
mkdir build
cd build
cmake ../ \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${deps_dir}" \
    -DCMAKE_PREFIX_PATH="${deps_dir}" \
    -DPPNF_BUILD_CPP=OFF \
    -DPPNF_BUILD_TESTS=OFF \
    -DPPNF_BUILD_PYTHON=ON
cmake --build . --target install --parallel 2
cd ..

# Run the Python test suite.
python -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()"

set +e
set +x
