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

Installation from source
^^^^^^^^^^^^^^^^^^^^^^^^
pygmo_plugin_nonfree has to be installed from source, pygmo_plugin_nonfree has the following dependencies:

* pagmo (i.e., the C++ headers of the pagmo library need to be installed before attempting
  to compile pygmo),
* pygmo (i.e., the C++ headers of pygmo need to be installed before attempting
  to compile pygmo_plugin_nonfree. these can be obtained activating the PAGMO_BUILD_PYGMO option of the pagmo build system),
* `Boost.Python <http://www.boost.org/doc/libs/1_63_0/libs/python/doc/html/index.html>`__
* `NumPy <http://www.numpy.org/>`__ (note that NumPy's development headers must be installed as well).

To build the module from source you need to **activate** the ``PAGMO_PLUGINS_NONFREE_BUILD_PYTHON`` cmake option.
Check carefully what Python version and what libraries/include paths are detected (in particular, on systems with multiple Python versions
it can happen that CMake detects the headers from a Python version and the Python library from another version).
The ``CMAKE_INSTALL_PREFIX`` variable will be used to construct the final location of headers and Python module after install.

When done, type (in your build directory):

.. code-block:: bash

   make install
