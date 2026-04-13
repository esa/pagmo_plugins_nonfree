Plugins to commercial solvers for pagmo / pygmo
===============================================

.. image:: ../sphinx/docs/images/pagmo_logo.png
   :target: https://esa.github.io/pagmo2/index.html
   :width: 12%

.. image:: ../sphinx/docs/images/algo.png
   :target: index.html
   :width: 12%

.. image:: ../sphinx/docs/images/prob.png
   :target: index.html
   :width: 12%

.. image:: ../sphinx/docs/images/pop.png
   :target: index.html
   :width: 12%

.. image:: ../sphinx/docs/images/island.png
   :target: index.html
   :width: 12%

.. image:: ../sphinx/docs/images/archi.png
   :target: index.html
   :width: 12%

.. image:: ../sphinx/docs/images/migration.png
   :target: index.html
   :width: 12%


``pagmo_plugins_nonfree`` is an affiliated package of
`pagmo/pygmo <https://esa.github.io/pagmo2/index.html>`_.

It adds commercial solvers to the list of User Defined Algorithms (UDAs).
All added UDAs are implemented as plugins, so third-party libraries are loaded
at run time. You must ensure you have access rights to the solver libraries.
Library name and path can be specified when constructing the UDAs.

Getting Started
^^^^^^^^^^^^^^^

.. toctree::
   :maxdepth: 1

   install
   quickstart

C++
^^^
API reference for C++ plugins.

.. toctree::
   :maxdepth: 1

   cpp_snopt7
   cpp_worhp


Python
^^^^^^
API reference for Python plugins.

.. toctree::
   :maxdepth: 1

   py_snopt7
   py_worhp
