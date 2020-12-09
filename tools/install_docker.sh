#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

PAGMO_LATEST="2.14.0"
PYBIND11_VERSION="2.5.0"


if [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *38 ]]; then
	PYTHON_DIR="cp38-cp38"
	PYTHON_VERSION="38"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *37 ]]; then
	PYTHON_DIR="cp37-cp37m"
	PYTHON_VERSION="37"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *36 ]]; then
	PYTHON_DIR="cp36-cp36m"
	PYTHON_VERSION="36"
else
	echo "Invalid build type: ${PAGMO_PLUGINS_NONFREE_BUILD}"
	exit 1
fi

cd
cd install

# Python mandatory deps.
/opt/python/${PYTHON_DIR}/bin/pip install cloudpickle numpy dill ipyparallel

# Install pybind11
curl -L https://github.com/pybind/pybind11/archive/v${PYBIND11_VERSION}.tar.gz > v${PYBIND11_VERSION}
tar xvf v${PYBIND11_VERSION} > /dev/null 2>&1
cd pybind11-${PYBIND11_VERSION}
mkdir build
cd build
cmake ../ -DPYBIND11_TEST=OFF > /dev/null
make install > /dev/null 2>&1
cd ..
cd ..

# pagmo
curl -L  https://github.com/esa/pagmo2/archive/v${PAGMO_LATEST}.tar.gz > pagmo2.tar.gz
tar xzf pagmo2.tar.gz
cd pagmo2-${PAGMO_LATEST}
mkdir build
cd build
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DPAGMO_WITH_EIGEN3=yes \
	-DPAGMO_WITH_NLOPT=yes \
	-DPAGMO_WITH_IPOPT=yes \
	-DCMAKE_BUILD_TYPE=Release ../;
make -j2 install
cd ../..

# pygmo
curl -L  https://github.com/esa/pygmo2/archive/v${PAGMO_LATEST}.tar.gz > pygmo2.tar.gz
tar xzf pygmo2.tar.gz
cd pygmo2-${PAGMO_LATEST}
mkdir build
cd build
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DPYTHON_EXECUTABLE=/opt/python/${PYTHON_DIR}/bin/python ../;
make -j2 install

# pygmo_plugins_nonfree
cd /pagmo_plugins_nonfree
mkdir build_pagmo_plugins_nonfree
cd build_pagmo_plugins_nonfree
cmake -DCMAKE_BUILD_TYPE=Release \
	-DBoost_NO_BOOST_CMAKE=ON \
	-DPPNF_BUILD_CPP=yes \
	-DPPNF_BUILD_TESTS=no \
	-DPPNF_BUILD_PYTHON=no ../;
make -j2 install
cd ..
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
	-DBoost_NO_BOOST_CMAKE=ON \
	-DPPNF_BUILD_CPP=no \
	-DPPNF_BUILD_PYTHON=yes \
	-DPYTHON_EXECUTABLE=/opt/python/${PYTHON_DIR}/bin/python ../;
make -j2 install 

cd wheel
# Copy the installed pygmo files, wherever they might be in /usr/local,
# into the current dir.
cp -a `find /usr/local/lib -type d -iname 'pygmo_plugins_nonfree'` ./
# Create the wheel and repair it.
/opt/python/${PYTHON_DIR}/bin/python setup.py bdist_wheel
auditwheel repair dist/pygmo_plugins_nonfree* -w ./dist2
# Try to install it and run the tests.
cd /
/opt/python/${PYTHON_DIR}/bin/pip install /pagmo_plugins_nonfree/build/wheel/dist2/pygmo_plugins_nonfree*
/opt/python/${PYTHON_DIR}/bin/python -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()";


# Upload to pypi. This variable will contain something if this is a tagged build (vx.y.z), otherwise it will be empty.
export PAGMO_PLUGINS_NONFREE_RELEASE_VERSION=`echo "${TRAVIS_TAG}"|grep -E 'v[0-9]+\.[0-9]+.*'|cut -c 2-`
if [[ "${PAGMO_PLUGINS_NONFREE_RELEASE_VERSION}" != "" ]]; then
    echo "Release build detected, uploading to PyPi."
    /opt/python/${PYTHON_DIR}/bin/pip install twine
	/opt/python/${PYTHON_DIR}/bin/twine upload -u ci4esa /pagmo_plugins_nonfree/build/wheel/dist2/pygmo_plugins_nonfree*
fi