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

Args:
   screen_output (``bool``): when True will activate the original screen output from SNOPT7 and deactivate the logging system based on
     :class:`~pygmo_snopt7.set_verbosity()`.
   absolute_lib_path (``str``): the absolute path to the snopt7_c library in your system

Raises:
   ArgumentError: for any conversion problems between the python types and the c++ signature

.. warning::

   Unfortunately, SNOPT7 fortran code is only available acquiring a licence.
   If you do have such a licence, then you will also have the fortran files and can build them into the library
   snopt7 (one single library). In what follows, we assume the snopt7 fortran library is available in your
   system. Since pagmo wraps around the C interface you will have to compile also the library snopt7_c, which is open
   source and can be obtained from https://github.com/snopt/snopt-interface. 

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

std::string snopt7_set_integer_option_docstring()
{
    return R"(set_integer_option(name, value)

Set integer option.

This method will set the optimisation integer option \p name to \p value.
The optimisation options are passed to the snOptA API when calling evolve().

Args:
   name (``string``): name of the option
   value (``int``): value of the option

The available integer options are listed in the following table:

==========================  ==============  ===================================================
Name                        Default Value   Notes
==========================  ==============  ===================================================
Objective row               1               has precedence over ObjRow (snOptA)
Verify level                0               Gradients Check Flag
Scale option                1               Scaling (1 - linear constraints and variables)
Crash option                3               3 - first basis is essentially triangular
Iterations limit            10000           or 20*ncons if that is more
Partial price               1               10 for large LPs
Major iterations limit      1000            or ncons if that is more
Minor iterations limit      500             or 3*ncons if that is more
Superbasics limit           None            n1 + 1, n1 = number of nonlinear variables
New superbasics limit       99              controls early termination of QPs
Proximal point method       1               1 - satisfies linear constraints near x0
Reduced Hessian dimension   2000            or Superbasics limit if that is less
Violation limit             10.0            unscaled constraint violation limit
Hessian frequency           999999          for full Hessian (never reset)
Hessian updates             10              for limited memory Hessian
Hessian flush               999999          no flushing
Check frequency             60              test row residuals l2norm(Ax - sk)
Expand frequency            10000           for anti-cycling procedure
Factorization frequency     50              100 for LPs
Save frequency              100             save basis map
Old basis file              0               input basis map
New basis file              0               output basis map
Backup basis file           0               output extra basis map
Insert file                 0               input in industry format
Punch file                  0               output Insert data
Load file                   0               input names and values
Dump file                   0               output Load data
Solution file               0               different from printed solution
Total character workspace   500             lencw: 500
Total integer workspace     None            leniw: 500 + 100 * (m+n)
Total real workspace        None            lenrw: 500 + 200 * (m+n)
User character workspace    500
User integer workspace      500
User real workspace         500
Debug level                 0               0 - Normal, 1 - for developers
Timing level                3               3 - print cpu times
==========================  ==============  ===================================================

.. note::

   In case of invalid option name this function will be correctly executed, but a subsequent call to evolve() will raise a ValueError.

Examples:
    >>> import pygmo as pg
    >>> import pygmo_plugins_nonfree as pg7
    >>> udp = pg.problem(pg.cec2006(prob_id = 1))
    >>> uda = pg7.snopt7(False, "/usr/local/lib/")
    >>> uda.set_integer_option("Iterations limit", 10)
    >>> algo = pg.algorithm(uda)
    >>> algo.set_verbosity(10)
    >>> pop = pg.population(udp,1)
    >>> pop = algo.evolve(pop) # doctest: +SKIP
    SNOPT7 plugin for pagmo/pygmo:
    The gradient sparsity is assumed dense: 130 components detected.
    The gradient is computed numerically by SNOPT7.
    <BLANKLINE>
     objevals:        objval:      violated:    viol. norm:
             1       -78.0445              8        105.847 i
            11       -78.0445              8        105.847 i
            21       -74.7751              8        100.505 i
            31       -74.7751              8        100.505 i
    <BLANKLINE>
    Resource limit error - iteration limit reached

)";
}

std::string snopt7_set_numeric_option_docstring()
{
    return R"(set_numeric_option(name, value)

Set numeric option.

This method will set the optimisation numeric option \p name to \p value.
The optimisation options are passed to the snOptA API when calling evolve().

Args:
   name (``string``): name of the option
   value (``int``): value of the option

The available numeric options are listed in the following table:

============================  ==============  ===================================================
Name                          Default Value   Notes
============================  ==============  ===================================================
Infinite bound                1.0e+20         Infinite Bound Value
Major feasibility tolerance   1.0e-6          Target Nonlinear Constraint Violation
Major optimality tolerance    1.0e-6          Target Complementarity Gap
Minor feasibility tolerance   1.0e-6          For Satisfying the QP Bounds
Scale tolerance               0.9             Scaling Tolerance
Crash tolerance               0.1
Linesearch tolerance          0.9             smaller for more accurate search
Pivot tolerance               3.7e-11         epsilon^{2/3}
Elastic weight                1.0e+4          used only during elastic mode
Major step limit              2.0
Function precision            3.0e-13         epsilon^0.8 (almost full accuracy)
Difference interval           5.5e-7          Function precision^(1/2)
Central difference interval   6.7e-5          Function precision^(1/3)
Penalty parameter             0.0             initial penalty parameter
Unbounded step size           1.0e+18
Unbounded objective           1.0e+15
LU factor tolerance           3.99            for NP (100.0 for LP)
LU update tolerance           3.99            for NP ( 10.0 for LP)
LU singularity tolerance      3.2e-11
============================  ==============  ===================================================

.. note::

   In case of invalid option name this function will not throw, but a subsequent call to evolve() will raise a ValueError.

Examples:
    >>> import pygmo as pg
    >>> import pygmo_plugins_nonfree as pg7
    >>> udp = pg.problem(pg.cec2006(prob_id = 1))
    >>> uda = pg7.snopt7(False, "/usr/local/lib/")
    >>> uda.set_numeric_option("Major feasibility tolerance", 1e-2)
    >>> algo = pg.algorithm(uda)
    >>> algo.set_verbosity(20)
    >>> pop = pg.population(udp,1)
    >>> pop = algo.evolve(pop) # doctest: +SKIP
    SNOPT7 plugin for pagmo/pygmo:
    The gradient sparsity is assumed dense: 130 components detected.
    The gradient is computed numerically by SNOPT7.
    <BLANKLINE>
     objevals:        objval:      violated:    viol. norm:
             1       -112.494              9        175.091 i
            21        -109.07              9        169.082 i
            41        -105.64              9        163.066 i
            61       -11.3869              1    2.00323e-09 i
            81       -11.3764              2    1.09242e-06 i
           101       -11.4844              4    3.09867e-06 i
           121       -11.4841              5    0.000267805 i
           <BLANKLINE>
    Finished successfully - optimality conditions satisfied

)";
}

} // namespace
