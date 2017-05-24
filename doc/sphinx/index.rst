SNOPT7 plugin for pagmo / pygmo
===============================

This is an affliated package of `pagmo/pygmo <https://esa.github.io/pagmo2/index.html>`_ adding the commercial
Sparse Nonlinear OPTimizer (SNOPT) to the list of solvers.

Unfortunately, SNOPT7 fortran code is only available acquiring a licence.
If you do have such a licence, then you will also have the fortran files and can build them into the library
snopt7 (one single library). Since this plugin wraps the C interface you will have to compile also  `the library snopt7_c <https://github.com/snopt/snopt-interface>`_, which is open
source. We ask that when building such a library you link it to the fortran library. You can achieve this modifying the snopt-interface build system
(e.g. using "LDFLAGS = -avoid-version -lsnopt7" in the Makefile).

Contents:

.. toctree::
   :maxdepth: 1

   cpp_snopt7
   py_snopt7
