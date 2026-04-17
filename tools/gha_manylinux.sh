#!/usr/bin/env bash

# Fail fast on command errors, undefined vars, and pipeline failures.
set -Eeuo pipefail
# Keep shell tracing enabled so CI logs show every step and argument.
set -x

# Required inputs from the workflow/container invocation.
: "${PPNF_BUILD_TYPE:?PPNF_BUILD_TYPE is required}"
: "${GITHUB_WORKSPACE:?GITHUB_WORKSPACE is required}"

# Basic context useful for debugging CI runs.
echo "PPNF_BUILD_TYPE: ${PPNF_BUILD_TYPE}"
echo "GITHUB_REF: ${GITHUB_REF:-<unset>}"
echo "GITHUB_WORKSPACE: ${GITHUB_WORKSPACE}"

# Preflight: list interpreters baked into the manylinux image.
# This makes Python-version mismatches obvious in the logs.
echo "Preflight: available Python installs under /opt/python"
if [[ -d /opt/python ]]; then
	ls -1 /opt/python || true
else
	echo "WARNING: /opt/python directory is missing"
fi

for expected_dir in cp311-cp311 cp312-cp312 cp313-cp313 cp314-cp314; do
	if [[ -x "/opt/python/${expected_dir}/bin/python" ]]; then
		echo "Found interpreter: /opt/python/${expected_dir}/bin/python"
	else
		echo "Missing interpreter: /opt/python/${expected_dir}/bin/python"
	fi
done

# Needed when running in GitHub Actions containers.
git config --global --add safe.directory "${GITHUB_WORKSPACE}"

# Map workflow build type to the manylinux Python ABI directory.
case "${PPNF_BUILD_TYPE}" in
	*314*) PYTHON_DIR="cp314-cp314" ;;
	*313*) PYTHON_DIR="cp313-cp313" ;;
	*312*) PYTHON_DIR="cp312-cp312" ;;
	*311*) PYTHON_DIR="cp311-cp311" ;;
	*)
		echo "Invalid build type '${PPNF_BUILD_TYPE}'. Supported: Python314, Python313, Python312, Python311"
		exit 1
		;;
esac

# Resolve python/pip/ipcluster binaries for the selected interpreter.
PYBIN="/opt/python/${PYTHON_DIR}/bin"
if [[ ! -x "${PYBIN}/python" ]]; then
	echo "Python executable not found at ${PYBIN}/python. Update the manylinux image for ${PYTHON_DIR}."
	exit 1
fi
echo "PYTHON_DIR: ${PYTHON_DIR}"

# The versions can be overridden from the workflow if needed.
PAGMO_VERSION_RELEASE="${PAGMO_VERSION_RELEASE:-2.19.1}"
PYGMO_PYPI_VERSION_SPEC="${PYGMO_PYPI_VERSION_SPEC:-2.19.8}"

# Lightweight system diagnostics to help compare amd64 vs arm runs.
echo "System diagnostics:"
uname -a || true
echo "Architecture: $(uname -m)"
echo "CPU count: $(nproc)"
free -h || true
if command -v lscpu >/dev/null 2>&1; then
	lscpu || true
fi
if [[ -f /proc/meminfo ]]; then
	echo "Top of /proc/meminfo:" && head -n 6 /proc/meminfo || true
fi

# Small helper that uses /usr/bin/time -v when available to record peak RSS.
run_time() {
	if command -v /usr/bin/time >/dev/null 2>&1; then
		/usr/bin/time -v "$@"
	else
		"$@"
	fi
}

# Tag builds consume released pagmo sources; non-tag builds use pagmo git HEAD.
if [[ "${GITHUB_REF:-}" == "refs/tags/v"* ]]; then
	echo "Tag build detected"
	PPNF_RELEASE_BUILD="yes"
else
	echo "Non-tag build detected"
	PPNF_RELEASE_BUILD="no"
fi

# Install packaging/build/test runtime dependencies in the selected Python.
"${PYBIN}/python" -m pip install --upgrade pip setuptools wheel
"${PYBIN}/python" -m pip install cloudpickle numpy "pygmo==${PYGMO_PYPI_VERSION_SPEC}"
"${PYBIN}/python" -m pip install dill==0.3.5.1 networkx ipyparallel scipy auditwheel

# Install BLAS/LAPACK dependencies needed by xtensor-blas consumers.
yum install -y openblas-devel lapack-devel || yum install -y openblas lapack


# Build and install pagmo2 (released tarball on tags, git HEAD otherwise).
INSTALL_ROOT="${INSTALL_ROOT:-/root/install}"
mkdir -p "${INSTALL_ROOT}"
cd "${INSTALL_ROOT}"
if [[ "${PPNF_RELEASE_BUILD}" == "yes" ]]; then
	curl -fsSL -o pagmo2.tar.gz "https://github.com/esa/pagmo2/archive/refs/tags/v${PAGMO_VERSION_RELEASE}.tar.gz"
	tar xzf pagmo2.tar.gz
	cd "pagmo2-${PAGMO_VERSION_RELEASE}"
else
	rm -rf pagmo2
	git clone --depth 1 https://github.com/esa/pagmo2.git
	cd pagmo2
fi

# Configure and install pagmo2 into the container's default prefix.
rm -rf build
mkdir -p build
cd build
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DPAGMO_WITH_EIGEN3=yes \
	-DPAGMO_WITH_NLOPT=yes \
	-DPAGMO_WITH_IPOPT=yes \
	-DPAGMO_ENABLE_IPO=OFF \
	-DCMAKE_BUILD_TYPE=Release ../
run_time cmake --build . --target install --parallel 2

# Build and install the pagmo_plugins_nonfree C++ library first.
cd "${GITHUB_WORKSPACE}"
rm -rf build_cpp
mkdir -p build_cpp
cd build_cpp
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DPPNF_BUILD_CPP=yes \
	-DPPNF_BUILD_TESTS=no \
	-DPPNF_BUILD_PYTHON=no ../
run_time cmake --build . --target install --parallel 2

# Then build and install pygmo_plugins_nonfree against selected Python.
cd "${GITHUB_WORKSPACE}"
rm -rf build
mkdir -p build
cd build
cmake -DBoost_NO_BOOST_CMAKE=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DPPNF_BUILD_CPP=no \
	-DPPNF_BUILD_PYTHON=yes \
	-DPython3_EXECUTABLE="${PYBIN}/python" ../
run_time cmake --build . --target install --parallel 2

# Build wheel from the wheel/ packaging directory and repair it for manylinux.
cd wheel
rm -rf pygmo_plugins_nonfree
SITE_PACKAGES_DIR="$("${PYBIN}/python" -c 'import site; print(site.getsitepackages()[0])')"
cp -r "${SITE_PACKAGES_DIR}/pygmo_plugins_nonfree" ./

# Keep compatibility with images exposing shared libs under /usr/local/lib64.
export LD_LIBRARY_PATH="/usr/local/lib64:/usr/local/lib"
run_time "${PYBIN}/python" setup.py bdist_wheel
auditwheel repair dist/pygmo_plugins_nonfree*.whl -w ./dist2
unset LD_LIBRARY_PATH

# Smoke-test the repaired wheel in a clean root context.
cd /
"${PYBIN}/python" -m pip install --force-reinstall "${GITHUB_WORKSPACE}/build/wheel/dist2/pygmo_plugins_nonfree"*.whl
# Keep at least 2 engines: tests can rely on parallel workers.
"${PYBIN}/ipcluster" start --daemonize=True -n 2
sleep 20
"${PYBIN}/python" -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()"

set +x
