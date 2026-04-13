.. _quickstart:

Quick start 
===========

.. contents::


C++
---

After following the :ref:`install` you will be able to compile
and run your first C++ pagmo_plugins_nonfree program. Let us have a 
look at a mininal cpp file and its corresponding CMakeLists.txt:

.. _getting_started_c++:

.. literalinclude:: docs/examples/getting_started.cpp
   :language: c++
   :linenos:

.. literalinclude:: docs/examples/CMakeLists.txt
   :language: cmake
   :linenos:

Make a folder and place there the C++ code into a ``getting_started.cpp`` text
file and the CMake code into a ``CMakeLists.txt`` text file. Then, 
from the command line, you can build and run the example with:

.. code-block:: bash

   $ mkdir build && cd build
   $ cmake ..
   $ cmake --build .

-----------------------------------------------------------------------

Python
------

If you have successfully installed pygmo_plugins_non_free following the :ref:`install` you can try the following script:

.. _getting_started_py:

.. literalinclude:: docs/examples/getting_started.py
   :language: python
   :linenos:

Place it into a ``getting_started.py`` text file and run it with:

.. code-block:: bash

   python getting_started.py

We recommend the use of Jupyter or Ipython to enjoy pygmo and its affiliated packages the most.
