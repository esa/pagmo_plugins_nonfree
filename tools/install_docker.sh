#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

CMAKE_VERSION="3.11.1"
EIGEN3_VERSION="3.3.4"
# The boost version needs to be synchd with that of pygmp pip package
BOOST_VERSION="1.67.0"
NLOPT_VERSION="2.4.2"
PAGMO_VERSION="2.10"

if [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *37 ]]; then
	PYTHON_DIR="cp37-cp37m"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *36 ]]; then
	PYTHON_DIR="cp36-cp36m"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *35 ]]; then
	PYTHON_DIR="cp35-cp35m"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *27 ]]; then
	PYTHON_DIR="cp27-cp27mu"
else
	echo "Invalid build type: ${PAGMO_PLUGINS_NONFREE_BUILD}"
	exit 1
fi

# HACK: for python 3.x, the include directory
# is called 'python3.xm' rather than just 'python3.x'.
# This confuses the build system of Boost.Python, thus
# we create a symlink to 'python3.x'.
cd /opt/python/${PYTHON_DIR}/include
PY_INCLUDE_DIR_NAME=`ls`
# If the include dir ends with 'm', create a symlink
# without the 'm'.
if [[ $PY_INCLUDE_DIR_NAME == *m ]]; then
	ln -s $PY_INCLUDE_DIR_NAME `echo $PY_INCLUDE_DIR_NAME|sed 's/.$//'`
fi

cd
cd install

# Python deps (numpy is needed to install pygmo)
/opt/python/${PYTHON_DIR}/bin/pip install cloudpickle numpy

# pagmo & pygmo
curl -L  https://github.com/esa/pagmo2/archive/v${PAGMO_VERSION}.tar.gz > pagmo2.tar.gz
tar xzf pagmo2.tar.gz
cd pagmo2-${PAGMO_VERSION}
mkdir build_pagmo
cd build_pagmo
cmake ../ -DBoost_NO_BOOST_CMAKE=ON -DPAGMO_WITH_EIGEN3=yes -DPAGMO_WITH_NLOPT=yes -DCMAKE_BUILD_TYPE=Release
make install
cd ..
mkdir build_pygmo 
cd build_pygmo
cmake -DCMAKE_BUILD_TYPE=Release -DBoost_NO_BOOST_CMAKE=ON -DPAGMO_BUILD_PYGMO=yes -DPAGMO_BUILD_PAGMO=no -DPYTHON_EXECUTABLE=/opt/python/${PYTHON_DIR}/bin/python ../;
make -j2 install

# pygmo_plugins_nonfree
cd /pagmo_plugins_nonfree
mkdir build_pagmo_plugins_nonfree
cd build_pagmo_plugins_nonfree
cmake -DCMAKE_BUILD_TYPE=Release ../;
make install
cd ..
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBoost_NO_BOOST_CMAKE=ON -DPPNF_BUILD_CPP=no -DPPNF_BUILD_PYTHON=yes -DPYTHON_EXECUTABLE=/opt/python/${PYTHON_DIR}/bin/python ../;
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
/opt/python/${PYTHON_DIR}/bin/python -c "import pygmo_plugins_nonfree; pygmo_plugins_nonfree.test.run_test_suite(1)"

# Upload to pypi. This variable will contain something if this is a tagged build (vx.y.z), otherwise it will be empty.
export PAGMO_PLUGINS_NONFREE_RELEASE_VERSION=`echo "${TRAVIS_TAG}"|grep -E 'v[0-9]+\.[0-9]+.*'|cut -c 2-`
if [[ "${PAGMO_PLUGINS_NONFREE_RELEASE_VERSION}" != "" ]]; then
    echo "Release build detected, uploading to PyPi."
    /opt/python/${PYTHON_DIR}/bin/pip install twine
	/opt/python/${PYTHON_DIR}/bin/twine upload -u ci4esa /pagmo_plugins_nonfree/build/wheel/dist2/pygmo_plugins_nonfree*
fi