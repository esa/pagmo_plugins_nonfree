.. _install:

Installation
============

.. contents::

.. _cpp_install:

C++ Library
-----------

Dependencies
^^^^^^^^^^^^

``pagmo_plugins_nonfree`` has the following **mandatory** build/runtime dependencies:

* a C++17 capable compiler,
* `CMake <https://cmake.org/>`__ 3.20 or later,
* `Boost <https://www.boost.org/>`__ 1.69 or later,
* `pagmo <https://esa.github.io/pagmo2/>`__ 2.19 or later.

The C++ component builds as a shared library (it is not a header-only library).

Build from source
^^^^^^^^^^^^^^^^^

After making sure dependencies are available on your system, clone the source tree:

.. code-block:: console

  $ git clone https://github.com/esa/pagmo_plugins_nonfree.git
  $ cd pagmo_plugins_nonfree

Then configure and install the C++ library:

.. code-block:: console

  $ mkdir build_cpp
  $ cd build_cpp
  $ cmake ../ -DPPNF_BUILD_CPP=ON -DPPNF_BUILD_PYTHON=OFF
  $ cmake --build . --target install

Useful CMake options include:

* ``CMAKE_BUILD_TYPE``: build type (e.g. ``Release``, ``Debug``),
* ``CMAKE_PREFIX_PATH``: additional dependency lookup paths,
* ``CMAKE_INSTALL_PREFIX``: installation prefix.

.. _py_install:

Python
------

Dependencies
^^^^^^^^^^^^

The Python module corresponding to ``pagmo_plugins_nonfree`` is called
``pygmo_plugins_nonfree``.

``pygmo_plugins_nonfree`` has the following **mandatory** runtime dependencies:

* `pygmo <https://esa.github.io/pygmo2/>`__ (version ``2.19.*``),
* `NumPy <https://numpy.org/>`__,
* `cloudpickle <https://github.com/cloudpipe/cloudpickle>`__, a package that extends Python's serialization
  capabilities.

Additionally, optional runtime dependencies used by some features/tests include:

* `dill <https://pypi.org/project/dill/>`__,
* `NetworkX <https://networkx.org/>`__,
* `ipyparallel <https://ipyparallel.readthedocs.io/>`__,
* `SciPy <https://scipy.org/>`__.

Packages
^^^^^^^^

There are various options for the installation of ``pygmo_plugins_nonfree``:

* `conda <https://conda.io/docs/>`__
* `pip <https://pip.pypa.io/en/stable/>`__
* installation from source.

Conda
^^^^^

Conda builds are available through conda-forge feedstocks. Please refer to the
relevant package pages for current availability and platform coverage.

pip
^^^

``pygmo_plugins_nonfree`` wheels are published on PyPI:

* `https://pypi.org/project/pygmo_plugins_nonfree/ <https://pypi.org/project/pygmo_plugins_nonfree/>`__

Install with:

.. code-block:: console

  $ pip install pygmo_plugins_nonfree

Current CI status (pip/manylinux)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project CI currently validates and builds the following configurations:

* Non-manylinux Python CI:
  Linux x86_64 (Python 3.12, 3.13, 3.14), Linux ARM64 (Python 3.14),
  macOS ARM64 (Python 3.14), Windows x86_64 (Python 3.14).
* Manylinux wheel CI:
  manylinux 2.28 x86_64 and aarch64 for Python 3.11, 3.12, 3.13.
* Manual PyPI publication:
  wheels are published via the ``Publish to PyPI (manual)`` workflow dispatch.

Installation from source
^^^^^^^^^^^^^^^^^^^^^^^^

To build ``pygmo_plugins_nonfree`` from source you need:

* the C++ library ``pagmo_plugins_nonfree`` installed first,
* `pybind11 <https://github.com/pybind/pybind11>`__,
* a compatible Python installation,
* ``pygmo``, ``NumPy`` and the other runtime dependencies listed above.

A typical build sequence is:

.. code-block:: console

  $ cd /path/to/pagmo_plugins_nonfree
  $ mkdir build
  $ cd build
  $ cmake ../ -DPPNF_BUILD_CPP=OFF -DPPNF_BUILD_PYTHON=ON
  $ cmake --build . --target install

Pay special attention to ``Python3_EXECUTABLE`` and ``CMAKE_PREFIX_PATH`` when
multiple Python or dependency installations are present.
