#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != manylinux* ]]; then
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    export PATH="$deps_dir/bin:$PATH"
fi

if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "ReleaseGCC" ]]; then
    cmake -DCMAKE_PREFIX_PATH=$deps_dir -DBoost_NO_BOOST_CMAKE=ON -DCMAKE_BUILD_TYPE=Release -DPPNF_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest -VV;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "DebugGCC" ]]; then
    cmake -DCMAKE_PREFIX_PATH=$deps_dir -DBoost_NO_BOOST_CMAKE=ON -DCMAKE_BUILD_TYPE=Debug -DPPNF_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest -VV;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == "CoverageGCC" ]]; then
    cmake -DCMAKE_PREFIX_PATH=$deps_dir -DBoost_NO_BOOST_CMAKE=ON -DCMAKE_BUILD_TYPE=Debug -DPPNF_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="--coverage -fuse-ld=gold" ../;
    make -j2 VERBOSE=1;
    ctest -VV;
    bash <(curl -s https://codecov.io/bash) -x gcov-5;
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == Python* ]]; then
    # We need this to be the directory where pybind11 was installed 
    # in the script install_deps.sh
    export PPNF_BUILD_DIR=`pwd`
    # Install pagmo_plugins_nonfree
    cmake \
        -DCMAKE_INSTALL_PREFIX=$deps_dir \
        -DCMAKE_PREFIX_PATH=$deps_dir \
        -DBoost_NO_BOOST_CMAKE=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DPPNF_BUILD_TESTS=no \
        -DPPNF_BUILD_CPP=yes \
        -DPPNF_BUILD_PYTHON=no \
        ..
    make install VERBOSE=1;
    # Install pygmo_plugins_nonfree.
    cmake \
        -DCMAKE_INSTALL_PREFIX=$deps_dir \
        -DCMAKE_PREFIX_PATH=$deps_dir \
        -DCMAKE_BUILD_TYPE=Release \
        -DPPNF_BUILD_TESTS=no \
        -DPPNF_BUILD_CPP=no \
        -DPPNF_BUILD_PYTHON=yes \
        ..
    make install VERBOSE=1;
    # Move out of the build dir.
    cd ../tools
    # Run the test suite
    python -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()";

    # Documentation.
    cd ../build
    # At the moment conda has these packages only for Python 3.4. Install via pip instead.
    pip install 'sphinx-bootstrap-theme';
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
    #export SPHINX_OUTPUT=`make html 2>&1 >/dev/null`;
    make html;
    #if [[ "${SPHINX_OUTPUT}" != "" ]]; then
    #    echo "Sphinx encountered some problem:";
    #    echo "${SPHINX_OUTPUT}";
    #    exit 1;
    #fi
    echo "Sphinx ran successfully";
    make doctest;
    if [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" != "Python37" ]]; then
        echo "${PAGMO_PLUGINS_NONFREE_BUILD} build detected, skipping the docs upload. Only available for Python37 builds";
        # Stop here. Docs are uploaded only in the Python37 build.
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
elif [[ "${PAGMO_PLUGINS_NONFREE_BUILD}" == manylinux* ]]; then
    cd ..;
    docker pull ${DOCKER_IMAGE};
    docker run --rm -e TWINE_PASSWORD -e PAGMO_PLUGINS_NONFREE_BUILD -e TRAVIS_TAG -v `pwd`:/pagmo_plugins_nonfree $DOCKER_IMAGE bash /pagmo_plugins_nonfree/tools/install_docker.sh
fi

set +e
set +x
