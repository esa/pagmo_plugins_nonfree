#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# For the non manylinux builds (i.e. pip) we use conda and thus install and activate a conda environment
if [[ "${PAGMO_BUILD}" != manylinux* ]]; then
    if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
        wget https://repo.continuum.io/miniconda/Miniconda2-latest-MacOSX-x86_64.sh -O miniconda.sh;
    else
        wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
    fi
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    bash miniconda.sh -b -p $HOME/miniconda
    conda config --add channels conda-forge --force

    # All builds will need these
    conda_pkgs="boost>=1.56 cmake>=3.2 pagmo>=2.0"

    # Only Python builds will need these
    if [[ "${PAGMO_BUILD}" == "Python36" || "${PAGMO_BUILD}" == "OSXPython36" ]]; then
        conda_pkgs="$conda_pkgs python=3.6 pygmo>=2.0"
    elif [[ "${PAGMO_BUILD}" == "Python27" || "${PAGMO_BUILD}" == "OSXPython27" ]]; then
        conda_pkgs="$conda_pkgs python=2.7 pygmo>=2.0"
    fi

    if [[ "${PAGMO_BUILD}" == Python* ]]; then
        conda_pkgs="$conda_pkgs graphviz doxygen"
    fi

    # We create the conda environment and activate it
    conda create -q -p $deps_dir -y $conda_pkgs
    source activate $deps_dir
fi

set +e
set +x