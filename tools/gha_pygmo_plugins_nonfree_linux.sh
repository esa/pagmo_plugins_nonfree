#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Mambaforge-Linux-x86_64.sh -O mambaforge.sh
export deps_dir=$HOME/local
export PATH="$HOME/mambaforge/bin:$PATH"
bash mambaforge.sh -b -p $HOME/mambaforge
mamba env create -f ppnf_devel.yml -q -p $deps_dir
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
