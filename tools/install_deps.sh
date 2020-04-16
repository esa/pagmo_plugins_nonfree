#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

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
        git clone https://github.com/pybind/pybind11.git
        cd pybind11
        git checkout 4f72ef846fe8453596230ac285eeaa0ce3278bb4
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