#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

PYBIND11_VERSION="2.6.0"

# For the non manylinux builds (i.e. pip) we use conda and thus install and activate a conda environment
if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != manylinux* ]]; then
    if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
        wget https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -O miniconda.sh;
    else
        wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh;
    fi
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    export PATH="$deps_dir/bin:$PATH"

    bash miniconda.sh -b -p $HOME/miniconda
    conda config --add channels conda-forge --force

    # All builds will need these
    conda_pkgs="boost cmake pagmo-devel cxx-compiler"

    # Only Python builds will need these
    if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "Python37" ]]; then
        conda_pkgs="$conda_pkgs python=3.7 pygmo" 
    fi

    if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == *Python* ]]; then
        conda_pkgs="$conda_pkgs graphviz doxygen sphinx breathe"
    fi

    # We create the conda environment and activate it
    conda create -q -p $deps_dir -y $conda_pkgs
    source activate $deps_dir

    # For python builds, we install pybind11 from the specific commit
    # needed to guarantee interoperability with pyaudi/pygmo
    if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == *Python* ]]; then
        export PPNF_BUILD_DIR=`pwd`
        curl -L https://github.com/pybind/pybind11/archive/v${PYBIND11_VERSION}.tar.gz > v${PYBIND11_VERSION}
        tar xvf v${PYBIND11_VERSION} > /dev/null 2>&1
        cd pybind11-${PYBIND11_VERSION}
        mkdir build
        cd build
        cmake \
            -DPYBIND11_TEST=NO \
            -DCMAKE_INSTALL_PREFIX=$PPNF_BUILD_DIR \
            -DCMAKE_PREFIX_PATH=$PPNF_BUILD_DIR \
            -DPYTHON_EXECUTABLE=$HOME/local/bin/python3 \
            ..
        make install
        cd ../..
    fi
fi

set +e
set +x