.. _quickstart:

Quick start 
============

.. contents::


C++
---

After following the :ref:`install` you will be able to compile and run your first C++ pagmo_plugins_nonfree program:

.. _getting_started_c++:

.. literalinclude:: docs/examples/getting_started.cpp
   :language: c++
   :linenos:

Place it into a ``getting_started.cpp`` text file and compile it (for example) with:

.. code-block:: bash

   g++ -I ~/.local/include -I /usr/include/eigen3/ -O2 -DNDEBUG -std=c++11 getting_started.cpp -pthread -lboost_system -lboost_filesystem -ldl

Note that we have made some assumptions in the above line. 1) your pagmo was installed with the eigen option activated, 2) you installed the headers
in a local folder in your user directory.

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

We recommend the use of Jupyter or Ipython to enjoy pygmo the most.
