#!/usr/bin/env bash

# Echo each command.
set -x

# Exit on error.
set -e

# Report on the environrnt variables used for this build
echo "PPNF_BUILD_TYPE: ${PPNF_BUILD_TYPE}"
echo "GITHUB_REF: ${GITHUB_REF}"
echo "GITHUB_WORKSPACE: ${GITHUB_WORKSPACE}"
# No idea why but this following line seems to be necessary (added: 18/01/2023)
git config --global --add safe.directory ${GITHUB_WORKSPACE}
BRANCH_NAME=`git rev-parse --abbrev-ref HEAD`
echo "BRANCH_NAME: ${BRANCH_NAME}"


# 1 - We read for what python wheels have to be built
if [[ ${PPNF_BUILD_TYPE} == *38* ]]; then
	PYTHON_DIR="cp38-cp38"
elif [[ ${PPNF_BUILD_TYPE} == *39* ]]; then
	PYTHON_DIR="cp39-cp39"
elif [[ ${PPNF_BUILD_TYPE} == *310* ]]; then
	PYTHON_DIR="cp310-cp310"
elif [[ ${PPNF_BUILD_TYPE} == *311* ]]; then
	PYTHON_DIR="cp311-cp311"
elif [[ ${PPNF_BUILD_TYPE} == *312* ]]; then
	PYTHON_DIR="cp312-cp312"
else
	echo "Invalid build type: ${PPNF_BUILD_TYPE}"
	exit 1
fi

# Report the inferred directory where python is found
echo "PYTHON_DIR: ${PYTHON_DIR}"

# The pagmo/pygmo versions to be used.
export PAGMO_VERSION="2.19.0"
export PYGMO_VERSION="2.19.5"

# Check if this is a release build.
if [[ "${GITHUB_REF}" == "refs/tags/v"* ]]; then
    echo "Tag build detected"
	export PPNF_RELEASE_BUILD="yes"
else
	echo "Non-tag build detected"
fi

# Python mandatory deps.
/opt/python/${PYTHON_DIR}/bin/pip install cloudpickle numpy pygmo==${PYGMO_VERSION}
# Python optional deps.
/opt/python/${PYTHON_DIR}/bin/pip install dill==0.3.5.1 networkx ipyparallel scipy

# In the pagmo2/manylinux228_x86_64_with_deps:latest image in dockerhub
# the working directory is /root/install, we will install pagmo there
cd /root/install

# Install pagmo
curl -L -o pagmo2.tar.gz https://github.com/esa/pagmo2/archive/refs/tags/v${PAGMO_VERSION}.tar.gz
tar xzf pagmo2.tar.gz
cd pagmo2-*

mkdir build
cd build
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DPAGMO_WITH_EIGEN3=yes \
	-DPAGMO_WITH_NLOPT=yes \
	-DPAGMO_WITH_IPOPT=yes \
	-DPAGMO_ENABLE_IPO=ON \
	-DCMAKE_BUILD_TYPE=Release ../;
make -j4 install

# pygmo_plugins_nonfree
cd ${GITHUB_WORKSPACE}
# NOTE: this is temporary.
cat boost_dll.diff | patch -p1
mkdir build_pagmo_plugins_nonfree
cd build_pagmo_plugins_nonfree
cmake -DCMAKE_BUILD_TYPE=Release \
	-DBoost_NO_BOOST_CMAKE=ON \
	-DPPNF_BUILD_CPP=yes \
	-DPPNF_BUILD_TESTS=no \
	-DPPNF_BUILD_PYTHON=no ../;
make -j4 install
cd ..
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
	-DBoost_NO_BOOST_CMAKE=ON \
	-DPPNF_BUILD_CPP=no \
	-DPPNF_BUILD_PYTHON=yes \
	-DPYTHON_EXECUTABLE=/opt/python/${PYTHON_DIR}/bin/python ../;
make -j4 install

# Making the wheel and installing it
cd wheel
# Move the installed ppnf files, wherever they might be in /usr/local,
# into the current dir.
mv `/opt/python/${PYTHON_DIR}/bin/python -c 'import site; print(site.getsitepackages()[0])'`/pygmo_plugins_nonfree* ./
# Create the wheel and repair it.
# NOTE: this is temporary because some libraries in the docker
# image are installed in lib64 rather than lib and they are
# not picked up properly by the linker.
export LD_LIBRARY_PATH="/usr/local/lib64:/usr/local/lib"
/opt/python/${PYTHON_DIR}/bin/python setup.py bdist_wheel
auditwheel repair dist/pygmo* -w ./dist2
# Try to install it and run the tests.
unset LD_LIBRARY_PATH
cd /
/opt/python/${PYTHON_DIR}/bin/pip install ${GITHUB_WORKSPACE}/build/wheel/dist2/pygmo*
/opt/python/${PYTHON_DIR}/bin/ipcluster start --daemonize=True
sleep 20
/opt/python/${PYTHON_DIR}/bin/python -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()";

# Upload to pypi. This variable will contain something if this is a tagged build (vx.y.z), otherwise it will be empty.
# if [[ "${PYGMO_RELEASE_VERSION}" != "" ]]; then
# 	echo "Release build detected, creating the source code archive."
# 	cd ${GITHUB_WORKSPACE}
# 	TARBALL_NAME=${GITHUB_WORKSPACE}/build/wheel/dist2/pygmo-${PYGMO_RELEASE_VERSION}.tar
# 	git archive --format=tar --prefix=pygmo2/ -o ${TARBALL_NAME} ${BRANCH_NAME}
# 	tar -rf ${TARBALL_NAME} --transform "s,^build/wheel/pygmo.egg-info,pygmo2," build/wheel/pygmo.egg-info/PKG-INFO
# 	gzip -9 ${TARBALL_NAME}
# 	echo "... uploading all to PyPi."
# 	/opt/python/${PYTHON_DIR}/bin/pip install twine
# 	/opt/python/${PYTHON_DIR}/bin/twine upload -u ci4esa ${GITHUB_WORKSPACE}/build/wheel/dist2/pygmo*
# fi

# Upload to PyPI.
if [[ "${PPNF_RELEASE_BUILD}" == "yes" ]]; then
	/opt/python/${PYTHON_DIR}/bin/pip install twine
	/opt/python/${PYTHON_DIR}/bin/twine upload -u ci4esa ${GITHUB_WORKSPACE}/build/wheel/dist2/pygmo*
fi

set +e
set +x
