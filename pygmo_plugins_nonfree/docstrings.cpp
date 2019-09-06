#include <string>

#include "docstrings.hpp"

namespace pygmo
{

std::string snopt7_docstring()
{
    return R"(__init__(screen_output = False, library = "/usr/local/lib/libsnopt7.so", minor_version=6)

SNOPT 7 - (Sparse Nonlinear OPTimizer, Version 7)

This class is a user-defined algorithm (UDA) that contains a plugin to the Sparse Nonlinear OPTimizer (SNOPT, V7)
solver, a software package for large-scale nonlinear optimization. SNOPT7 is a powerful solver that is able to handle
robustly and efficiently constrained nonlinear opimization problems also at high dimensionalities.

Intended use::
   >>> import pygmo as pg
   >>> import pygmo_plugins_nonfree as ppnf
   >>> uda = ppnf.snopt7(screen_output = False, library = "/usr/local/lib/libsnopt76.so", minor_version = 6)
   >>> algo = pg.algorithm(uda)

SNOPT7 supports only single-objective minimisation, using a sequential quadratic programming (SQP) algorithm.
Search directions are obtained from QP subproblems that minimize a quadratic model of the Lagrangian function
subject to linearized constraints. An augmented Lagrangian merit function is reduced along each search
direction to ensure convergence from any starting point.

In order to support pagmo's population-based optimisation model, *snopt7* selects
a single individual from the input *population* to be optimised.
If the optimisation produces an improved individual (as established by pagmo comparison criteria),
the optimised individual will be inserted back into the population.

Args:
   screen_output (``bool``): when True will activate the original screen output from SNOPT7 and deactivate the logging system based on
     :class:`~pygmo_snopt7.set_verbosity()`.
   library (``str``): the snopt7 library filename in your system (absolute path included)
   minor_version (``int``): The minor version of your Snopt7 library. Only two APIs are supported at the moment: 
     7.6 and 7.7. You may try to use this plugin with different minor version numbers, but at your own risk.

Raises:
   ArgumentError: for any conversion problems between the python types and the c++ signature

.. note::

   SNOPT7 fortran code is only available acquiring a licence.
   If you do have such a licence, then you will also have the fortran files and can build them into the library
   snopt7 (one single library). The library snopt7_c will then need to be built,
   compiling the correct release of the project https://github.com/snopt/snopt-interface. The library thus created
   will link to your fortran snopt7 library. As an alternative you may have only one library libsnopt7 containing
   both the Fortran and the C interface (this is the case, for example, of the library you can download for evaluation).

.. note::

   This plugin was tested with snopt version 7.2 as well as with the compiled evaluation libraries (7.7)
   made available via the snopt7 official web site (C/Fortran library).

.. warning::

   Constructing this class with an inconsistent *minor_version* parameter results in undefined behaviour.

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
   of how the ``"Major feasibility tolerance"`` numeric option is handled in :class:`pygmo.snopt7` and the note below.

.. note::

   The definitions of feasibility are different for SNOPT7 and pygmo. SNOPT7 requires that *max(c_viol)/||x|| <= eps_r*
   where *||x||* is the Euclidean norm of *x*, a candidate solution vector, and *eps_r* is the "Major feasibility tolerance" option in SNOPT7. 
   In contrast, pygmo requires that *c_viol <= c_tol* where *c_viol* 
   is the vector of absolute values of the nonlinear constraint violations and *c_tol* is the vector of constraint tolerances in pygmo.problem. To guarantee
   feasibility with respect to pygmo when SNOPT7 reports feasibility, try setting *eps_r <= min(c_tol)/||x||_ub*, where
   *||x||_ub* is an upper bound on the value of *||x||*. Care must be taken with this approach to ensure *eps_r* is not too small.

.. note::

   Snopt7 supports its own logging format and protocol, including the ability to print to screen and write to file.
   Snopt7's screen logging is disabled by default. It can be activated upon construction by setting the relative kwarg to ``True``

Examples:
    >>> from pygmo import *
    >>> from pygmo_plugins_nonfree import snopt7
    >>> algo = algorithm(snopt7(screen_output = False, library = "/usr/local/lib/libsnopt7.so"))
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
    >>> uda = pg7.snopt7(False, "/usr/local/lib/libsnopt7.so")
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
   value (``float``): value of the option

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
    >>> uda = pg7.snopt7(False, "/usr/local/lib/libsnopt7.so")
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

std::string worhp_docstring()
{
    return R"(__init__(screen_output = false, library = '\usr\local\lib\libworhp.so')

WORHP - (We Optimize Really Huge Problems)

This class is a user-defined algorithm (UDA) that contains a plugin to the WORHP (We Optimize Really Huge Problems)
solver, a software package for large-scale nonlinear optimization. WORHP is a powerful solver that is able to handle
robustly and efficiently constrained nonlinear opimization problems also at high dimensionalities. The wrapper
was developed around the version 1.12 of WORHP and the Full Feature Interface (FFI) using the Unified Solver
Interface and the Reverse Communication paradigm (see worhp user manual).

Intended use::
   >>> import pygmo as pg
   >>> import pygmo_plugins_nonfree as ppnf
   >>> uda = ppnf.worhp(screen_output = False, library = "/usr/local/lib/libworhp.so")
   >>> algo = pg.algorithm(uda)

.. note::

   The WORHP library is only available buying a licence. You can consult the web pages at
   (https://worhp.de/) for further information. There you will be able to download the correct library for your
   architecture and obtain a license file. You will be able to specify the location of the downloaded library when
   constructing this UDA.

Worhp is designed to efficiently solve small- to large-scale constrained optimisation problems, where
the objective function and the constraints are sufficiently smooth, and may be linear, quadratic or nonlinear. It is
designed to find locally optimal points of optimisation problems, which may be globally optimal, depending on the
problem structure, the initial guess and other factors. Worhp combines  a  Sequential  Quadratic  Programming  (SQP)
method  on  the general nonlinear level with a primal-dual Interior Point (IP) method on the quadratic subproblem
level, to generate a sequence of search directions, which are subject to line search using the Augmented Lagrangian
or L1 merit function.

Worhp needs first and second order derivatives, which can be supplied by the user, or approximated by finite
differences or quasi-Newton methods.

In order to support pagmo's population-based optimisation model, *snopt7* selects
a single individual from the input *population* to be optimised.
If the optimisation produces an improved individual (as established by pagmo comparison criteria),
the optimised individual will be inserted back into the population.

Args:
   screen_output (``bool``): when True will activate the original screen output from SNOPT7 and deactivate the logging system based on
     :class:`~pygmo_snopt7.set_verbosity()`.
   library (``str``): the worhp library filename in your system (absolute path included)

Raises:
   ArgumentError: for any conversion problems between the python types and the c++ signature

.. note::

   This plugin for the WORHP was developed around version 1.12.1 of the worhp library. It will not 
   work with versions having a different amajor or minor version.

.. warning::

   A moved-from :cpp:class:`pagmo::worhp` is destructible and assignable. Any other operation will result
   in undefined behaviour.

.. seealso::

   https://worhp.de/

See also the docs of the C++ class :cpp:class:`worhp::worhp`.

)";
}

std::string worhp_get_log_docstring()
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
   is, they might not necessarily be consistent with worhp's notion of feasibility. 

.. note::

   WORHP supports its own screen output. It can be activated upon construction by setting the relative kwarg to ``True``

Examples:
    >>> from pygmo import *
    >>> from pygmo_plugins_nonfree import worhp
    >>> algo = algorithm(worhp(screen_output = False, library = "/usr/local/lib/libworhp.so"))
    >>> algo.set_verbosity(1)
    >>> prob = problem(cec2006(prob_id = 1))
    >>> prob.c_tol = [1e-6]*9
    >>> pop = population(prob, 1)
    >>> pop = algo.evolve(pop) # doctest: +SKIP
     Error (Read_XML_File): Could not open file param.xml.
     WorhpFromXML: Could not open parameter file, using default values.
    WORHP version is (library): 1.12.1
    WORHP version is (plugin headers): 1.12.1
    <BLANKLINE>
    WORHP plugin for pagmo/pygmo: 
        The gradient sparsity is assumed dense: 130 components detected.
        The gradient is computed numerically by WORHP.
        The hessian of the lagrangian sparsity has: 91 components.
        The hessian of the lagrangian is computed numerically by WORHP.
    <BLANKLINE>
    The following parameters have been set by pagmo to values other than their xml provided ones (or their default ones): 
        par.FGtogether: true
        par.UserDF: false
        par.UserDG: false
        par.UserHM: false
        par.TolFeas: false
        par.AcceptTolFeas: false
    <BLANKLINE>
    objevals:        objval:      violated:    viol. norm:
           13       -57.4531              9        69.6602 i
           26       -57.4525              9        69.6591 i
           39       -57.4531              9        69.6602 i
           52       -57.4525              9        69.6591 i
           65       -43.2785              9        45.1295 i
           78       -43.2783              9        45.1293 i
           91       -22.0316              6        10.7489 i
          104       -22.0315              6        10.7488 i
          117        -15.149              6       0.208663 i
          130       -15.1489              6       0.208673 i
          143       -15.0001              6     5.0492e-05 i
          156            -15              6    6.80956e-05 i
          169            -15              0              0
          182            -15              1    8.99984e-06 i
    <BLANKLINE>
    Warning (ParameterReset): WORHP has reset 2 invalid parameters to default values.
    <BLANKLINE>
     Final values after iteration 5:
     Final objective value ............. -1.4999999999E+01
     Final constraint violation ........  0.0000000000E+00
     Final complementarity .............  0.0000000000E+00 (0.0000000000E+00)
     Final KKT conditions ..............  1.2589097553E-11 (4.3087122652E-06)
     Successful termination: Optimal Solution Found.
    <BLANKLINE>
    <BLANKLINE>
    >>> uda = algo.extract(worhp)
    >>> uda.get_log() # doctest: +SKIP
    [(1, -214.45104382308432, 9, 294.79616317933454, False), (11, -214.45108700799688, ...      
)";
}

std::string worhp_set_numeric_option_docstring()
{
    return R"(set_numeric_option(name, value)

Set numeric option.

This method will set the optimisation numeric option \p name to \p value.
The optimisation options are passed to the worhp API when calling evolve().

Args:
   name (``string``): name of the option
   value (``float``): value of the option

.. note::

   In case of invalid option name this function will not throw, but a subsequent call to evolve() will raise a ValueError.

)";
}

std::string worhp_set_integer_option_docstring()
{
    return R"(set_integer_option(name, value)

Set integer option.

This method will set the optimisation integer option \p name to \p value.
The optimisation options are passed to the worhp API when calling evolve().

Args:
   name (``string``): name of the option
   value (``int``): value of the option

.. note::

   In case of invalid option name this function will not throw, but a subsequent call to evolve() will raise a ValueError.

)";
}

std::string worhp_set_bool_option_docstring()
{
    return R"(set_bool_option(name, value)

Set bool option.

This method will set the optimisation boolean option \p name to \p value.
The optimisation options are passed to the worhp API when calling evolve().

Args:
   name (``string``): name of the option
   value (``bool``): value of the option

.. note::

   In case of invalid option name this function will not throw, but a subsequent call to evolve() will raise a ValueError.

)";
}

} // namespace pygmo
