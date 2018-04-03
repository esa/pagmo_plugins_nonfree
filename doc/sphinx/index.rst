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


``pagmo_plugins_nonfree`` is an affliated package of `pagmo/pygmo <https://esa.github.io/pagmo2/index.html>`_. It adds some 
commercial solvers to the list of User Defined Algorithms (UDAs). All the added UDAs are developed as plugins, that is, they
work loading the third party library at run-time. The user needs to make sure to have the library and the right to use it. 
The library name and path can be specified upon construction of the UDAs.

Contents:

.. toctree::
   :maxdepth: 1

   install
   quickstart

C++
^^^
Contents:

.. toctree::
   :maxdepth: 1

   cpp_snopt7
   cpp_worhp


Python
^^^^^^
Contents:

.. toctree::
   :maxdepth: 1

   py_snopt7
   py_worhp
