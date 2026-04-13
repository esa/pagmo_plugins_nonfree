# PowerShell script
$ErrorActionPreference = "Stop"

$depsDir = $env:CONDA_PREFIX
if (-not $depsDir) {
    throw "CONDA_PREFIX is not set"
}

Write-Host "CONDA_PREFIX: $depsDir"

conda config --set always_yes yes
conda config --add channels conda-forge
conda config --set channel_priority strict
conda install cmake ninja c-compiler cxx-compiler pagmo-devel "libboost-devel>=1.86" pybind11 pybind11-abi pygmo

# Build and install the C++ library.
if (Test-Path build_cpp) { Remove-Item -Recurse -Force build_cpp }
New-Item -ItemType Directory -Path build_cpp | Out-Null
Push-Location build_cpp
cmake -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="$depsDir" `
    -DCMAKE_PREFIX_PATH="$depsDir" `
    ..
cmake --build . --config Release --target INSTALL --parallel 2
Pop-Location

# Build and install the Python extension.
if (Test-Path build) { Remove-Item -Recurse -Force build }
New-Item -ItemType Directory -Path build | Out-Null
Push-Location build
cmake -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="$depsDir" `
    -DCMAKE_PREFIX_PATH="$depsDir" `
    -DPPNF_BUILD_CPP=OFF `
    -DPPNF_BUILD_TESTS=OFF `
    -DPPNF_BUILD_PYTHON=ON `
    ..
cmake --build . --config Release --target INSTALL --parallel 2
Pop-Location

python -c "import pygmo_plugins_nonfree; import pygmo; pygmo_plugins_nonfree.test.run_test_suite(1); pygmo.mp_bfe.shutdown_pool()"
