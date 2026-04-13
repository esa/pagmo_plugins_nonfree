#!/usr/bin/env bash

# Echo each command.
set -x

# Exit on error.
set -e

DEPS_DIR="$HOME/local"

# Install build and docs deps into a dedicated conda environment.
conda create -y -q -p "$DEPS_DIR" \
    c-compiler cxx-compiler cmake ninja \
    pagmo-devel "libboost-devel>=1.86" pybind11 pybind11-abi pygmo \
    python=3.13 \
    "sphinx=4.5.0" sphinx-bootstrap-theme breathe "doxygen<1.13" graphviz

# Build and install the C++ library.
conda run -p "$DEPS_DIR" cmake -S . -B build_cpp -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$DEPS_DIR" \
    -DCMAKE_INSTALL_PREFIX="$DEPS_DIR"
conda run -p "$DEPS_DIR" cmake --build build_cpp --target install --parallel 2

# Build and install the Python extension.
conda run -p "$DEPS_DIR" cmake -S . -B build_py -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$DEPS_DIR" \
    -DCMAKE_INSTALL_PREFIX="$DEPS_DIR" \
    -DPPNF_BUILD_CPP=OFF \
    -DPPNF_BUILD_PYTHON=ON
conda run -p "$DEPS_DIR" cmake --build build_py --target install --parallel 2

# Build Doxygen XML consumed by Sphinx/Breathe.
(
    cd doc/doxygen
    conda run -p "$DEPS_DIR" doxygen
    cp images/* xml/
)

# Build HTML docs and soft-fail on external link flakiness.
(
    cd doc/sphinx
    conda run -p "$DEPS_DIR" make html
    if ! conda run -p "$DEPS_DIR" make linkcheck; then
        echo "Warning: Sphinx linkcheck failed (likely transient external outage); continuing."
    fi
)

set +e
set +x
