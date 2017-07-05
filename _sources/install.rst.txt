.. _install:

Installation guide
==================

.. contents::

.. _cpp_install:

C++
---

pagmo_plugins_nonfree is a header-only library which has the following third-party dependencies:

* `Boost <http://www.boost.org/>`__, **mandatory**, version 1.61.0 minimum. Headers and the compiled libraries boost_system and boost_filesystem are needed. 
* `pagmo <https://esa.github.io/pagmo2/index.html>`__, **mandatory**, header-only

After making sure the dependencies above are installed in your system, you can download the
pagmo source code from the `GitHub release page <https://github.com/esa/pagmo_plugins_nonfree/releases>`__. Alternatively,
and if you like living on the bleeding edge, you can get the very latest version of pagmo via the ``git``
command:

.. code-block:: bash

   git clone https://github.com/esa/pagmo_plugins_nonfree.git

Once you are in pagmo_plugin_nonfree's source tree, you must configure your build using ``cmake``. This will allow
you to enable support for optional dependencies, configure the install destination, etc. When done,
you can install pagmo_plugin_nonfree via the command

.. code-block:: bash

   make install

The headers will be installed in the ``CMAKE_INSTALL_PREFIX/include`` directory. 

.. _py_install:

Python
------

The Python module corresponding to pagmo_plugin_nonfree is called pygmo_plugin_nonfree. pygmo_plugin_nonfree has some runtime Python dependencies:

* `pygmo <https://esa.github.io/pagmo2/index.html>`__, the pygmo package
* `NumPy <http://www.numpy.org/>`__, the standard Python array library
* `cloudpickle <https://github.com/cloudpipe/cloudpickle>`__, a package that extends Python's serialization
  capabilities.

There are various options for the installation of pygmo_plugin_nonfree:

* `conda <https://conda.io/docs/>`__
* `pip <https://pip.pypa.io/en/stable/>`__
* installation from source.

The following table summarizes the pros and cons of the various installation methods:

========= ============ ============ ========== ========== ================ ==========
Method    Linux Py 2.7 Linux Py 3.x OSX Py 2.7 OSX Py 3.x Win Py 2.7       Win Py 3.x
========= ============ ============ ========== ========== ================ ==========
conda     64bit        64bit        64bit      64bit      ✘                64bit
pip       64bit        64bit        ✘          ✘          64bit (MinGW)    64bit (MinGW)
source    32/64bit     32/64bit     32/64bit   32/64bit   32/64bit (MinGW) 32/64bit
========= ============ ============ ========== ========== ================ ==========

In general, we recommend the use of `conda <https://conda.io/docs/>`__: in addition to making the installation
of pygmo easy, it also provides user-friendly access to a wealth of packages from the scientific Python
ecosystem. Conda is a good default choice in Linux and OSX.

In order to provide a better experience to our Windows users, we also publish `pip <https://pip.pypa.io/en/stable/>`__
packages for pygmo built with `MinGW <https://mingw-w64.org/doku.php>`__. The pip packages are also available on
Linux for those users who might prefer pip to conda, but they are **not** available on OSX.

Finally, it is always possible to compile pygmo_plugin_nonfree from source. This is the most flexible and powerful option, but of course
also the least user-friendly.

Installation from source
^^^^^^^^^^^^^^^^^^^^^^^^
pygmo_plugin_nonfree has the following dependencies:

* pagmo (i.e., the C++ headers of the pagmo library need to be installed before attempting
  to compile pygmo),
* pygmo (i.e., the C++ headers of pygmo need to be installed before attempting
  to compile pygmo_plugin_nonfree. these can be obtained activating the PAGMO_BUILD_PYGMO option of the pagmo build system),
* `Boost.Python <http://www.boost.org/doc/libs/1_63_0/libs/python/doc/html/index.html>`__
* `Boost.System <http://www.boost.org/doc/libs/1_63_0/libs/system/doc/index.html>`__
* `Boost.Filesystem <http://www.boost.org/doc/libs/1_63_0/libs/filesystem/doc/index.htm>`__
* `NumPy <http://www.numpy.org/>`__ (note that NumPy's development headers must be installed as well).

To build the module from source you need to **activate** the ``PAGMO_PLUGINS_NONFREE_BUILD_PYTHON`` cmake option.
Check carefully what Python version and what libraries/include paths are detected (in particular, on systems with multiple Python versions
it can happen that CMake detects the headers from a Python version and the Python library from another version).
The ``CMAKE_INSTALL_PREFIX`` variable will be used to construct the final location of headers and Python module after install.

When done, type (in your build directory):

.. code-block:: bash

   make install
