#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

PAGMO_VERSION="2.12.0"

if [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *37 ]]; then
	PYTHON_DIR="cp37-cp37m"
	BOOST_PYTHON_LIBRARY_NAME="libboost_python37.so"
	PYTHON_VERSION="37"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *36 ]]; then
	PYTHON_DIR="cp36-cp36m"
	BOOST_PYTHON_LIBRARY_NAME="libboost_python36.so"
	PYTHON_VERSION="36"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *27mu ]]; then
	PYTHON_DIR="cp27-cp27mu"
	BOOST_PYTHON_LIBRARY_NAME="libboost_python27mu.so"
	PYTHON_VERSION="27"
elif [[ ${PAGMO_PLUGINS_NONFREE_BUILD} == *27 ]]; then
	PYTHON_DIR="cp27-cp27m"
	BOOST_PYTHON_LIBRARY_NAME="libboost_python27.so"
	PYTHON_VERSION="27"
else
	echo "Invalid build type: ${PAGMO_PLUGINS_NONFREE_BUILD}"
	exit 1
fi

cd
cd install

# Python mandatory deps.
/opt/python/${PYTHON_DIR}/bin/pip install cloudpickle numpy
# Python optional deps.
if [[ ${PAGMO_PLUGINS_NONFREE_BUILD} != *27m ]]; then
	# NOTE: do not install the optional deps for the py27m build: some of the deps
	# don't have binary wheels available for py27m, which makes pip try to
	# install them from source (which fails).
	/opt/python/${PYTHON_DIR}/bin/pip install dill ipyparallel
	#/opt/python/${PYTHON_DIR}/bin/ipcluster start --daemonize=True
	#sleep 20
fi

# pagmo & pygmo
curl -L  https://github.com/esa/pagmo2/archive/v${PAGMO_VERSION}.tar.gz > pagmo2.tar.gz
tar xzf pagmo2.tar.gz
cd pagmo2-${PAGMO_VERSION}
mkdir build_pagmo
cd build_pagmo
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DPAGMO_WITH_EIGEN3=yes \
	-DPAGMO_WITH_NLOPT=yes \
	-DPAGMO_WITH_IPOPT=yes \
	-DCMAKE_BUILD_TYPE=Release ../;
make -j2 install
cd ../
mkdir build_pygmo
cd build_pygmo
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DPAGMO_BUILD_PYGMO=yes \
	-DPAGMO_BUILD_PAGMO=no \
	-DBoost_PYTHON${PYTHON_VERSION}_LIBRARY_RELEASE=/usr/local/lib/${BOOST_PYTHON_LIBRARY_NAME} \
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
	-DBoost_PYTHON${PYTHON_VERSION}_LIBRARY_RELEASE=/usr/local/lib/${BOOST_PYTHON_LIBRARY_NAME} \
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
/opt/python/${PYTHON_DIR}/bin/python -c "import pygmo_plugins_nonfree; pygmo_plugins_nonfree.test.run_test_suite(1)"

# Upload to pypi. This variable will contain something if this is a tagged build (vx.y.z), otherwise it will be empty.
export PAGMO_PLUGINS_NONFREE_RELEASE_VERSION=`echo "${TRAVIS_TAG}"|grep -E 'v[0-9]+\.[0-9]+.*'|cut -c 2-`
if [[ "${PAGMO_PLUGINS_NONFREE_RELEASE_VERSION}" != "" ]]; then
    echo "Release build detected, uploading to PyPi."
    /opt/python/${PYTHON_DIR}/bin/pip install twine
	/opt/python/${PYTHON_DIR}/bin/twine upload -u ci4esa /pagmo_plugins_nonfree/build/wheel/dist2/pygmo_plugins_nonfree*
fi