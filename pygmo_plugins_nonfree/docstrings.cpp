#include <string>

#include "docstrings.hpp"

namespace pygmo
{

std::string snopt7_docstring()
{
    return R"(__init__(screen_output = false, absolute_lib_path = "\usr\local\lib\")

SNOPT 7 - (Sparse Nonlinear OPTimizer, Version 7)

This class is a user-defined algorithm (UDA) that contains a plugin to the Sparse Nonlinear OPTimizer (SNOPT, V7)
solver, a software package for large-scale nonlinear optimization. SNOPT7 is a powerful solver that is able to handle
robustly and efficiently constrained nonlinear opimization problems also at high dimensionalities.

SNOPT7 supports only single-objective minimisation, using a sequential quadratic programming (SQP) algorithm.
Search directions are obtained from QP subproblems that minimize a quadratic model of the Lagrangian function
subject to linearized constraints. An augmented Lagrangian merit function is reduced along each search
direction to ensure convergence from any starting point.

In order to support pagmo's population-based optimisation model, :cpp:class:`pagmo::snopt7` will select
a single individual from the input pagmo::population to be optimised.
If the optimisation produces an improved individual (as established by pagmo::compare_fc()),
the optimised individual will be inserted back into the population.
The selection and replacement strategies can be configured via set_selection(const std::string &),
set_selection(population::size_type), set_replacement(const std::string &) and
set_replacement(population::size_type).

.. warning::

   Unfortunately, SNOPT7 fortran code is only available acquiring a licence.
   If you do have such a licence, then you will also have the fortran files and can build them into the library
   snopt7 (one single library). In what follows, we assume the snopt7 fortran library is available in your
   system. Since pagmo wraps around the C interface you will have to compile also the library snopt7_c, which is open
   source and can be obtained from https://github.com/snopt/snopt-interface. We ask that when building such a library
   you link it to the fortran library. You can achieve this modifying the snopt-interface build system
   (e.g. using "LDFLAGS = -avoid-version -lsnopt7" in the Makefile).

.. note::

   We developed this plugin for the SNOPT version 7.6, but nothing significant has changed in the fortran
   files since the old days. As a consequence, as long as your C library has the symbols snInit, setIntParameter,
   setRealParameter, deleteSNOPT and solveA this plugin will work also with older SNOPT versions.

.. warning::

   A moved-from :cpp:class:`pagmo::snopt7` is destructible and assignable. Any other operation will result
   in undefined behaviour.

.. warning::

   The possibility to exploit the linear part of the problem fitness, part of the original SNOPT7 library,
   is deactivated in this plugin for pagmo.

.. seealso::

   https://web.stanford.edu/group/SOL/snopt.htm.

.. seealso::

   https://web.stanford.edu/group/SOL/guides/sndoc7.pdf.

Args:
   screen_output (``bool``): when True will activate the original screen output from SNOPT7 and deactivate the logging system based on
     :class:`~pygmo_snopt7.set_verbosity()`.
   absolute_lib_path (``str``): the absolute path to the snopt7_c library in your system (remember that it needs to link to the snopt7 fortran library)

Raises:
   ArgumentError: for any conversion problems between the python types and the c++ signature

See also the docs of the C++ class :cpp:class:`pagmo::snopt7`.

)";
}

std::string snopt7_get_log_docstring()
{
    return R"(get_log()

Returns:
    ``list``: the optimisation log containing the values ``objevals``, ``objval``, ``violated``, ``viol. norm``, ``feas.``, where:

    * ``objevals`` (``int``), the number of objective function evaluations made so far
    * ``objval`` (``float``), the objective function value for the current decision vector
    * ``violated`` (``int``), the number of constraints violated by the current decision vector
    * ``viol. norm`` (``float``), the constraints violation norm for the current decision vector
    * ``feas.`` (``bool``), a boolean flag signalling the feasibility of the current decision vector (as determined by pagmo)

Raises:
    unspecified: any exception thrown by failures at the intersection between C++ and Python (e.g.,
    type conversion errors, mismatched function signatures, etc.)

.. warning::

   The number of constraints violated, the constraints violation norm and the feasibility flag stored in the log
   are all determined via the facilities and the tolerances specified within :class:`pygmo.problem`. That
   is, they might not necessarily be consistent with Snopt's notion of feasibility. See the explanation
   of how the ``"Major feasibility tolerance"`` numeric option is handled in :class:`pygmo.snopt7`.

.. note::

   Snopt7 supports its own logging format and protocol, including the ability to print to screen and write to file.
   Snopt7's screen logging is disabled by default. It can be activated upon construction by setting the relative kwarg to ``True``

Examples:
    >>> from pygmo import *
    >>> from pygmo_snopt7 import snopt7
    >>> algo = algorithm(snopt7(screen_output = False, absolute_lib_path = "/usr/local/lib/"))
    >>> algo.set_verbosity(1)
    >>> prob = problem(cec2006(prob_id = 1))
    >>> prob.c_tol = [1e-6]*9
    >>> pop = population(prob, 1)
    >>> pop = algo.evolve(pop) # doctest: +SKIP
    SNOPT7 plugin for pagmo/pygmo: 
    The gradient sparsity is assumed dense: 130 components detected.
    The gradient is computed numerically by SNOPT7.
    <BLANKLINE>
    objevals:        objval:      violated:    viol. norm:
            1       -214.451              9        294.796 i
           11       -214.451              9        294.796 i
           21       -210.962              9        289.599 i
           31       -207.466              9        284.397 i
           41       -207.466              9        284.397 i
           51       -207.469              9        284.397 i
           61        -14.802              0              0
           71        -14.802              1    9.50602e-08 i
           81        -14.798              1    1.04126e-07 i
           91            -15              0              0
          101            -15              1    9.54841e-08 i
          111       -14.9987              0              0
          121       -14.9997              1    0.000534611 i
          131       -14.9997              0              0
    <BLANKLINE>
    Finished successfully - optimality conditions satisfied
    <BLANKLINE>
    >>> uda = algo.extract(snopt7)
    >>> uda.get_log() # doctest: +SKIP
    [(1, -214.45104382308432, 9, 294.79616317933454, False), (11, -214.45108700799688, ...

)";
}

} // namespace