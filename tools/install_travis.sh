#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# If needed we make sure conda stuff is found
if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != manylinux* ]]; then
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    export PATH="$deps_dir/bin:$PATH"
fi

if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "CoverageGCC5" ]]; then
    CXX=g++-5 CC=gcc-5 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="--coverage -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
    bash <(curl -s https://codecov.io/bash) -x gcov-5;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugGCC6" ]]; then
    CXX=g++-6 CC=gcc-6 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "ReleaseClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "OSXDebug" ]]; then
    CXX=clang++ CC=clang cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-g0 -O2" ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "OSXRelease" ]]; then
    CXX=clang++ CC=clang cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DPAGMO_PLUGINS_NONFREE_BUILD_TESTS=yes ../;
    make -j2 VERBOSE=1;
    ctest;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == Python* ]]; then
    export CXX=g++-4.8
    export CC=gcc-4.8
    # Install pygmo_plugins_nonfree.
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_PYTHON=yes -DPAGMO_PLUGINS_NONFREE_HEADERS=no ../;
    make install VERBOSE=1;
    ipcluster start --daemonize=True;
    # Give some time for the cluster to start up. The cluster may be needed in tests of archipelagos with ipyparallel islands
    sleep 20;
    # Move out of the build dir.
    cd ../tools
    # Run the test suite
    python -c "import pygmo_plugins_nonfree as pg7; pg7.test.run_test_suite(1)";

    # Documentation.
    cd ../build
    # At the moment conda has these packages only for Python 3.4. Install via pip instead.
    pip install 'sphinx<1.6' 'breathe<4.12' requests[security] 'sphinx-bootstrap-theme<0.5';
    # Run doxygen and check the output.
    cd ../doc/doxygen;
    export DOXYGEN_OUTPUT=`doxygen 2>&1 >/dev/null`;
    if [[ "${DOXYGEN_OUTPUT}" != "" ]]; then
        echo "Doxygen encountered some problem:";
        echo "${DOXYGEN_OUTPUT}";
        exit 1;
    fi
    echo "Doxygen ran successfully";
    # Copy the images into the xml output dir (this is needed by sphinx).
    cp images/* xml/;
    cd ../sphinx/;
    export SPHINX_OUTPUT=`make html 2>&1 >/dev/null`;
    if [[ "${SPHINX_OUTPUT}" != "" ]]; then
        echo "Sphinx encountered some problem:";
        echo "${SPHINX_OUTPUT}";
        exit 1;
    fi
    echo "Sphinx ran successfully";
    make doctest;
    if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != "Python36" ]]; then
        echo "${PAGMO_PLUGINS_NONFREE_BUILD} build detected, skipping the docs upload. Only available for Python36 builds";
        # Stop here. Docs are uploaded only in the Python36 build.
        exit 0;
    fi
    if [[ "${TRAVIS_PULL_REQUEST}" != "false" ]]; then
        echo "Testing a pull request, the generated documentation will not be uploaded.";
        exit 0;
    fi
    if [[ "${TRAVIS_BRANCH}" != "master" ]]; then
        echo "Branch is not master, the generated documentation will not be uploaded.";
        exit 0;
    fi
    # Move out the resulting documentation.
    mv _build/html /home/travis/sphinx;
    # Checkout a new copy of the repo, for pushing to gh-pages.
    cd ../../../;
    git config --global push.default simple
    git config --global user.name "Travis CI"
    git config --global user.email "darioizzo@gmail.com"
    set +x
    git clone "https://${GH_TOKEN}@github.com/esa/pagmo_plugins_nonfree.git" pagmo_plugins_nonfree_gh_pages -q
    set -x
    cd pagmo_plugins_nonfree_gh_pages
    git checkout -b gh-pages --track origin/gh-pages;
    git rm -fr *;
    mv /home/travis/sphinx/* .;
    git add *;
    # We assume here that a failure in commit means that there's nothing
    # to commit.
    git commit -m "Update Sphinx documentation, commit ${TRAVIS_COMMIT} [skip ci]." || exit 0
    PUSH_COUNTER=0
    until git push -q
    do
        git pull -q
        PUSH_COUNTER=$((PUSH_COUNTER + 1))
        if [ "$PUSH_COUNTER" -gt 3 ]; then
            echo "Push failed, aborting.";
            exit 1;
        fi
    done
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == OSXPython* ]]; then
    export CXX=clang++
    export CC=clang
    # Install pygmo_plugins_nonfree.
    cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DPAGMO_PLUGINS_NONFREE_BUILD_PYTHON=yes -DPAGMO_PLUGINS_NONFREE_HEADERS=no ../;
    make install VERBOSE=1;
    ipcluster start --daemonize=True;
    # Give some time for the cluster to start up.
    sleep 20;
    # Move out of the build dir.
    cd ../tools
    python -c "import pygmo_plugins_nonfree as pg7; pg7.test.run_test_suite(1)";
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == manylinux* ]]; then
    cd ..;
    docker pull ${DOCKER_IMAGE};
    docker run --rm -e TWINE_PASSWORD -e PAGMO_PLUGINS_NONFREE_BUILD -e TRAVIS_TAG -v `pwd`:/pagmo_plugins_nonfree $DOCKER_IMAGE bash /pagmo_plugins_nonfree/tools/install_docker.sh
fi

set +e
set +x
