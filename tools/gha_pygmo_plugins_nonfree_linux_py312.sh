#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget

# Install conda+deps.
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge
conda config --set channel_priority strict

conda create -y -q -p $deps_dir python=3.12 c-compiler cxx-compiler ninja cmake>=3.18 pagmo-devel libboost-devel pybind11 pybind11-abi pygmo
source activate $deps_dir

# We build and install pagmo_plugins_nonfree
mkdir build_cpp
cd build_cpp
cmake -G "Ninja" ../ -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPPNF_BUILD_CPP=yes -DPPNF_BUILD_TESTS=no -DPPNF_BUILD_PYTHON=no -DBoost_NO_BOOST_CMAKE=ON
cmake --build . --target=install --config=Release -- -j 2

# We build and install pygmo_plugins_nonfree
cd ../
mkdir build
cd build
cmake -G "Ninja" ../ -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPPNF_BUILD_CPP=no -DPPNF_BUILD_TESTS=no -DPPNF_BUILD_PYTHON=yes -DBoost_NO_BOOST_CMAKE=ON
cmake --build . --target=install --config=Release -- -j 2

# We get out of build as to test the global installation
cd /
python -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()"

set +e
set +x
