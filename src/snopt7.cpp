/* Copyright 2018 PaGMO development team
This file is part of "pagmo plugins nonfree", a PaGMO affiliated library.
The "pagmo plugins nonfree" library, is free software;
you can redistribute it and/or modify it under the terms of either:
  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.
or
  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.
or both in parallel, as here.

Linking "pagmo plugins nonfree" statically or dynamically with other modules is
making a combined work based on "pagmo plugins nonfree". Thus, the terms and conditions
of the GNU General Public License cover the whole combination.

As a special exception, the copyright holders of "pagmo plugins nonfree" give you
permission to combine ABC program with free software programs or libraries that are
released under the GNU LGPL and with independent modules that communicate with
"pagmo plugins nonfree" solely through the interface defined by the headers included in
"pagmo plugins nonfree" bogus_libs folder.
You may copy and distribute such a system following the terms of the licence
for "pagmo plugins nonfree" and the licenses of the other code concerned, provided that
you include the source code of that other code when and as the "pagmo plugins nonfree" licence
requires distribution of source code and provided that you do not modify the interface defined in the bogus_libs folder

Note that people who make modified versions of "pagmo plugins nonfree" are not obligated to grant this special
exception for their modified versions; it is their choice whether to do so.
The GNU General Public License gives permission to release a modified version without this exception;
this exception also makes it possible to release a modified version which carries forward this exception.
If you modify the interface defined in the bogus_libs folder, this exception does not apply to your
modified version of "pagmo plugins nonfree", and you must remove this exception when you distribute your modified
version.

This exception is an additional permission under section 7 of the GNU General Public License, version 3 (“GPLv3”)

The "pagmo plugins nonfree" library, and its affiliated librares are distributed in the hope
that they will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.
You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the "pagmo plugins nonfree" library.  If not,
see https://www.gnu.org/licenses/. */

#include <algorithm> // std::min_element
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <exception>
#include <iomanip>
#include <limits> // std::numeric_limits
#include <mutex>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/not_population_based.hpp>
#include <pagmo/config.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/io.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/utils/constrained.hpp>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits> // std::false_type
#include <unordered_map>
#include <vector>

#include <pagmo_plugins_nonfree/snopt7.hpp>

extern "C" {
#include "../include/pagmo_plugins_nonfree/bogus_libs/snopt7_c_lib/snopt7_c.h"
}

// MINGW-specific warnings.
#if defined(__GNUC__) && defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
#pragma GCC diagnostic ignored "-Wsuggest-attribute=const"
#endif

namespace ppnf
{
namespace detail
{
// We use this to ensure deleteSNOPT is called also if exceptions occur.
template <typename snProblem>
struct sn_problem_raii {
    sn_problem_raii(snProblem *p, char *a, char *b, int n,
                    std::function<void(snProblem *, char *, char *, int)> &snInit,
                    std::function<void(snProblem *)> &deleteSNOPT)
        : m_prob(p), m_deleteSNOPT(deleteSNOPT)
    {
        snInit(p, a, b, n);
    }
    ~sn_problem_raii()
    {
        m_deleteSNOPT(m_prob);
    }
    snProblem *m_prob;
    std::function<void(snProblem *)> &m_deleteSNOPT;
};

inline void snopt_fitness_wrapper(int *Status, int *n, double x[], int *needF, int *nF, double F[], int *needG,
                                  int *neG, double G[], char cu[], int *lencu, int iu[], int *leniu, double ru[],
                                  int *lenru)
{
    (void)n;
    (void)cu;
    (void)lencu;
    (void)ru;
    (void)lenru;
    (void)leniu;
    // First we recover the info we have hidden in the workspace
    auto &info = *(static_cast<detail::user_data *>(static_cast<void *>(iu)));
    auto &verb = info.m_verbosity;
    auto &log = info.m_log;
    auto &f_count = info.m_objfun_counter;
    auto &p = info.m_prob;
    auto &dv = info.m_dv;
    // We copy the decision vector into the vector_double
    std::copy(x, x + p.get_nx(), dv.begin());
    // We try to call the UDP fitness and gradient
    try {
        if (*needF > 0) {
            auto fit = p.fitness(dv);
            for (size_t i = 0u; i < static_cast<size_t>(*nF); ++i) {
                F[i] = fit[i];
            }

            if (verb && !(f_count % verb)) {
                // Constraints bits.
                const auto ctol = p.get_c_tol();
                const auto c1eq
                    = pagmo::detail::test_eq_constraints(fit.data() + 1, fit.data() + 1 + p.get_nec(), ctol.data());
                const auto c1ineq = pagmo::detail::test_ineq_constraints(
                    fit.data() + 1 + p.get_nec(), fit.data() + fit.size(), ctol.data() + p.get_nec());
                // This will be the total number of violated constraints.
                const auto nv = p.get_nc() - c1eq.first - c1ineq.first;
                // This will be the norm of the violation.
                const auto l = c1eq.second + c1ineq.second;
                // Test feasibility.
                const auto feas = p.feasibility_f(fit);

                if (!(f_count / verb % 50u)) {
                    // Every 50 lines print the column names.
                    pagmo::print("\n", std::setw(10), "objevals:", std::setw(15), "objval:", std::setw(15),
                                 "violated:", std::setw(15), "viol. norm:", '\n');
                }
                // Print to screen the log line.
                pagmo::print(std::setw(10), f_count + 1u, std::setw(15), fit[0], std::setw(15), nv, std::setw(15), l,
                             feas ? "" : " i", '\n');
                // Record the log.
                log.emplace_back(f_count + 1u, fit[0], nv, l, feas);
            }

            // Update the counter.
            ++f_count;
        }

        if (*needG > 0 && p.has_gradient()) {
            auto grad = p.gradient(dv);
            for (size_t i = 0u; i < static_cast<size_t>(*neG); ++i) {
                G[i] = grad[i];
            }
        }
    } catch (...) {
        *Status = -100; // signals to snopt7 that things went south and it should stop.
        info.m_eptr = std::current_exception();
    }
}
namespace
{
std::vector<char> s_to_C(const std::string &in)
{
    std::vector<char> retval(in.begin(), in.end());
    retval.push_back('\0');
    return retval;
}
/// Type for the map containing the association between then snopt7 results and their textual description
const std::unordered_map<int, std::string> results
    = {{0, "None"},
       {1, "Finished successfully - optimality conditions satisfied"},
       {2, "Finished successfully - feasible point found"},
       {3, "Finished successfully - requested accuracy could not be achieved"},
       {5, "Finished successfully - elastic objective minimized"},
       {6, "Finished successfully - elastic infeasibilities minimized"},
       {11, "The problem appears to be infeasible - infeasible linear constraints"},
       {12, "The problem appears to be infeasible - infeasible linear equality constraints"},
       {13, "The problem appears to be infeasible - nonlinear infeasibilities minimized"},
       {14, "The problem appears to be infeasible - linear infeasibilities minimized"},
       {15, "The problem appears to be infeasible - infeasible linear constraints in QP subproblem"},
       {16, "The problem appears to be infeasible - infeasible nonelastic constraints"},
       {21, "The problem appears to be unbounded - unbounded objective"},
       {22, "The problem appears to be unbounded - constraint violation limit reached"},
       {31, "Resource limit error - iteration limit reached"},
       {32, "Resource limit error - major iteration limit reached"},
       {33, "Resource limit error - the superbasics limit is too small"},
       {34, "Resource limit error - time limit reached"},
       {41, "Terminated after numerical difficulties - current point cannot be improved"},
       {42, "Terminated after numerical difficulties - singular basis"},
       {43, "Terminated after numerical difficulties - cannot satisfy the general constraints"},
       {44, "Terminated after numerical difficulties - ill-conditioned null-space basis"},
       {45, "Terminated after numerical difficulties - unable to compute acceptable LU factors"},
       {51, "Error in the user-supplied functions - incorrect objective derivatives"},
       {52, "Error in the user-supplied functions - incorrect constraint derivatives"},
       {56, "Error in the user-supplied functions - irregular or badly scaled problem functions"},
       {61, "Undefined user-supplied functions - undefined function at the first feasible point"},
       {62, "Undefined user-supplied functions - undefined function at the initial point"},
       {63, "Undefined user-supplied functions - unable to proceed into undefined region"},
       {71, "User requested termination - terminated during function evaluation"},
       {74, "User requested termination - terminated from monitor routine"},
       {81, "Insufficient storage allocated - work arrays must have at least 500 elements"},
       {82, "Insufficient storage allocated - not enough character storage"},
       {83, "Insufficient storage allocated - not enough integer storage"},
       {84, "Insufficient storage allocated - not enough real storage"},
       {91, "Input arguments out of range - invalid input argument"},
       {92, "Input arguments out of range - basis file dimensions do not match this problem"},
       {141, "System error - wrong number of basic variables"},
       {142, "System error - error in basis package"}};

std::mutex library_load_mutex;
} // namespace
} // namespace detail

snopt7::snopt7(bool screen_output, std::string snopt7_c_library, unsigned minor_version)
    : m_snopt7_c_library(snopt7_c_library), m_minor_version(minor_version), m_integer_opts(), m_numeric_opts(),
      m_screen_output(screen_output), m_verbosity(0), m_log()
{
}

/// Evolve population.
/**
 * This method will select an individual from \p pop, optimise it using snOptA interface, replace an individual
 * in \p pop with the optimised individual, and finally return \p pop.
 * The individual selection and replacement criteria can be set via set_selection(const std::string &),
 * set_selection(population::size_type), set_replacement(const std::string &) and
 * set_replacement(population::size_type). The SNOPT7 solver will then run until one of the stopping criteria
 * is satisfied, and the return status of the SNOPT7 solver will be recorded (it can be fetched with
 * get_last_opt_result()).
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. warning::
 *
 *    All options passed to the snOptA interface are those set by the user via the ppnf::snopt7 interface, or
 *    where no user specifications are available, to the default detailed on the User Manual available online but
 *    with the following exception: "Major feasibility tolerance" is set to the default value 1E-6 or to the minimum
 *    among the values returned by pagmo::problem::get_c_tol() if not zero.
 *
 * .. note::
 *
 *    The definitions of feasibility are different for SNOPT7 and pagmo. SNOPT7 requires that *max(c_viol)/||x|| <=
 *    eps_r* where *||x||* is the Euclidean norm of *x*, a candidate solution vector, and *eps_r* is the "Major
 *    feasibility tolerance" option in SNOPT7. In contrast, pagmo requires that *c_viol <= c_tol* where *c_viol* is the
 *    vector of absolute values of the nonlinear constraint violations and *c_tol* is the vector of constraint
 *    tolerances in pagmo::problem. To guarantee feasibility with respect to pagmo when SNOPT7 reports feasibility, try
 *    setting *eps_r <= min(c_tol)/||x||_ub*, where *||x||_ub* is an upper bound on the value of *||x||*. Care must be
 *    taken with this approach to ensure *eps_r* is not too small.
 *
 * .. seealso::
 *
 *    https://www-leland.stanford.edu/group/SOL/guides/sndoc7.pdf
 *
 * \endverbatim
 *
 * @param pop the population to be optimised.
 *
 * @return the optimised population.
 *
 * @throws std::invalid_argument in the following cases:
 * - the population's problem is multi-objective or stochastic
 * @throws unspecified any exception thrown by the public interface of pagmo::problem or
 * pagmo::not_population_based.
 */
pagmo::population snopt7::evolve(pagmo::population pop) const
{
    if (m_minor_version > 6) {
        return evolve_version<snProblem_77>(pop);
    } else {
        return evolve_version<snProblem_76>(pop);
    }
}

/// Set verbosity.
/**
 * This method will set the algorithm's verbosity. If \p n is zero, no output is produced during the
 * optimisation and no logging is performed. If \p n is nonzero, then every \p n objective function evaluations the
 * status of the optimisation will be both printed to screen and recorded internally. See snopt7::log_line_type and
 * snopt7::log_type for information on the logging format. The internal log can be fetched via get_log().
 *
 * @param n the desired verbosity level.
 *
 * Example (verbosity 1):
 * @code{.unparsed}
 * objevals:        objval:      violated:    viol. norm:
 *         1        48.9451              1        1.25272 i
 *         2         30.153              1       0.716591 i
 *         3        26.2884              1        1.04269 i
 *         4        14.6958              2        7.80753 i
 *         5        14.7742              2        5.41342 i
 *         6         17.093              1      0.0905025 i
 *         7        17.1772              1      0.0158448 i
 *         8        17.0254              2      0.0261289 i
 *         9        17.0162              2     0.00435195 i
 *        10        17.0142              2    0.000188461 i
 *        11         17.014              1    1.90997e-07 i
 *        12         17.014              0              0
 * @endcode
 * The ``i`` at the end of some rows indicates that the decision vector is infeasible. Feasibility
 * is checked against the problem's tolerance.
 *
 * By default, the verbosity level is zero.
 *
 * \verbatim embed:rst:leading-asterisk
 * .. warning::
 *
 *    The number of constraints violated, the constraints violation norm and the feasibility flag stored in the
 *    log are all determined via the facilities and the tolerances specified within :cpp:class:`pagmo::problem`.
 *    That is, they might not necessarily be consistent with Snopt7's notion of feasibility.
 *
 * .. note::
 *
 *    Snopt7 supports its own logging format and protocol, including the ability to print to screen and write to
 *    file. Snopt7's screen logging is disabled by default. On-screen logging can be enabled constructing the
 *    object ppnf::snopt7 passing ``True`` as argument. In this case verbosity will not be allowed to be set.
 *
 * \endverbatim
 *
 */
void snopt7::set_verbosity(unsigned n)
{
    if (m_screen_output && n != 0u) {
        pagmo_throw(std::invalid_argument,
                    "Cannot set verbosity to a >0 value if SNOPT7 screen output is choosen (i.e. did "
                    "you construct this using True as argument?)");
    } else {
        m_verbosity = n;
    }
}
/// Get the optimisation log.
/**
 * See snopt7::log_type for a description of the optimisation log. Logging is turned on/off via
 * set_verbosity().
 *
 * @return a const reference to the log.
 */
const snopt7::log_type &snopt7::get_log() const
{
    return m_log;
}
/// Gets the verbosity level
/**
 * @return the verbosity level
 */
unsigned int snopt7::get_verbosity() const
{
    return m_verbosity;
}
/// Algorithm name
/**
 * One of the optional methods of any user-defined algorithm (UDA).
 *
 * @return a string containing the algorithm name
 */
std::string snopt7::get_name() const
{
    return "SNOPT7";
}
/// Get extra information about the algorithm.
/**
 * @return a human-readable string containing useful information about the algorithm's properties
 * (e.g., the SNOPT7 user-set options, the selection/replacement policies, etc.), the snopt7_c library path
 */
std::string snopt7::get_extra_info() const
{
    std::ostringstream ss;
    pagmo::stream(ss, "\tName of the snopt7_c library: ", m_snopt7_c_library);
    pagmo::stream(ss, "\n\tLibrary version declared: 7.", m_minor_version);

    if (!m_screen_output) {
        pagmo::stream(ss, "\n\tScreen output: (pagmo/pygmo) - verbosity ", std::to_string(m_verbosity));
    } else {
        pagmo::stream(ss, "\n\tScreen output: (snopt7)");
    }
    pagmo::stream(ss, "\n\tLast optimisation return code: ", detail::results.at(m_last_opt_res));
    pagmo::stream(ss, "\n\tIndividual selection ");
    if (boost::any_cast<pagmo::population::size_type>(&m_select)) {
        pagmo::stream(ss, "idx: ", std::to_string(boost::any_cast<pagmo::population::size_type>(m_select)));
    } else {
        pagmo::stream(ss, "policy: ", boost::any_cast<std::string>(m_select));
    }
    pagmo::stream(ss, "\n\tIndividual replacement ");
    if (boost::any_cast<pagmo::population::size_type>(&m_replace)) {
        pagmo::stream(ss, "idx: ", std::to_string(boost::any_cast<pagmo::population::size_type>(m_replace)));
    } else {
        pagmo::stream(ss, "policy: ", boost::any_cast<std::string>(m_replace));
    }
    if (m_integer_opts.size()) {
        pagmo::stream(ss, "\n\tInteger options: ", pagmo::detail::to_string(m_integer_opts));
    }
    if (m_numeric_opts.size()) {
        pagmo::stream(ss, "\n\tNumeric options: ", pagmo::detail::to_string(m_numeric_opts));
    }
    pagmo::stream(ss, "\n");
    return ss.str();
}

/// Set integer option.
/**
 * This method will set the optimisation integer option \p name to \p value.
 * The optimisation options are passed to the snOptA API when calling evolve().
 *
 * @param name of the option.
 * @param value of the option.
 */
void snopt7::set_integer_option(const std::string &name, int value)
{
    m_integer_opts[name] = value;
}
/// Set integer options.
/**
 * This method will set the optimisation integer options contained in \p m.
 * It is equivalent to calling set_integer_option() passing all the name-value pairs in \p m
 * as arguments.
 *
 * @param m the name-value map that will be used to set the options.
 */
void snopt7::set_integer_options(const std::map<std::string, int> &m)
{
    for (const auto &p : m) {
        set_integer_option(p.first, p.second);
    }
}
/// Get integer options.
/**
 * @return the name-value map of optimisation integer options.
 */
std::map<std::string, int> snopt7::get_integer_options() const
{
    return m_integer_opts;
}
/// Set numeric option.
/**
 * This method will set the optimisation numeric option \p name to \p value.
 * The optimisation options are passed to the snOptA API when calling evolve().
 *
 * @param name of the option.
 * @param value of the option.
 */
void snopt7::set_numeric_option(const std::string &name, double value)
{
    m_numeric_opts[name] = value;
}
/// Set numeric options.
/**
 * This method will set the optimisation numeric options contained in \p m.
 * It is equivalent to calling set_numeric_option() passing all the name-value pairs in \p m
 * as arguments.
 *
 * @param m the name-value map that will be used to set the options.
 */
void snopt7::set_numeric_options(const std::map<std::string, double> &m)
{
    for (const auto &p : m) {
        set_numeric_option(p.first, p.second);
    }
}
/// Get numeric options.
/**
 * @return the name-value map of optimisation numeric options.
 */
std::map<std::string, double> snopt7::get_numeric_options() const
{
    return m_numeric_opts;
}
/// Clear all integer options.
void snopt7::reset_integer_options()
{
    m_integer_opts.clear();
}
/// Clear all numeric options.
void snopt7::reset_numeric_options()
{
    m_numeric_opts.clear();
}
/// Get the result of the last optimisation.
/**
 * @return the result of the last call to snOptA, or 0 if no optimisations have been
 * run yet. The meaning of the code returned is detailed in the Snopt7 User Manual available
 * on line.
 * \verbatim embed:rst:leading-asterisk
 *
 * .. seealso::
 *
 *    https://www-leland.stanford.edu/group/SOL/guides/sndoc7.pdf
 *
 * \endverbatim
 */
int snopt7::get_last_opt_result() const
{
    return m_last_opt_res;
}

// This is the evolve which will be version dependent via the template argument (snProblem declaration is)
template <typename snProblem>
pagmo::population snopt7::evolve_version(pagmo::population &pop) const
{
    // We store some useful properties
    const auto &prob = pop.get_problem(); // This is a const reference, so using set_seed, for example, will not work
    auto dim = prob.get_nx();
    const auto bounds = prob.get_bounds();
    const auto &lb = bounds.first;
    const auto &ub = bounds.second;

    // PREAMBLE-------------------------------------------------------------------------------------------------
    // We start by checking that the problem is suitable for this particular algorithm.
    if (prob.get_nobj() != 1u) {
        pagmo_throw(std::invalid_argument, "Multiple objectives detected in " + prob.get_name() + " instance. "
                                               + get_name() + " cannot deal with them");
    }
    if (prob.is_stochastic()) {
        pagmo_throw(std::invalid_argument,
                    "The problem appears to be stochastic " + get_name() + " cannot deal with it");
    }

    if (!pop.size()) {
        // In case of an empty pop, just return it.
        return pop;
    }
    // ---------------------------------------------------------------------------------------------------------

    // ------------------------- SNOPT7 PLUGIN (we attempt loading the snopt7 library at run-time)--------------
    // We first declare the prototypes of the functions used from the library
    std::function<void(snProblem *, char *, char *, int)> snInit;
    std::function<int(snProblem *, char[], int)> setIntParameter;
    std::function<int(snProblem *, char[], double)> setRealParameter;
    std::function<void(snProblem *)> deleteSNOPT;
    std::function<int(snProblem *, int, int, int, double, int, snFunA, int, int *, int *, double *, int, int *, int *,
                      double *, double *, double *, double *, double *, int *, double *, double *, int *, double *,
                      int *, int *, double *)>
        solveA;
    // We then try to load the library at run time and locate the symbols used.
    try {
        // Here we import at runtime the snopt7_c library and protect the whole try block with a mutex
        std::lock_guard<std::mutex> lock(detail::library_load_mutex);
        boost::filesystem::path path_to_lib(m_snopt7_c_library);
        if (!boost::filesystem::is_regular_file(path_to_lib)) {
            pagmo_throw(std::invalid_argument, "The snopt7_c library path was constructed to be: "
                                                   + path_to_lib.string() + " and it does not appear to be a file");
        }
        boost::dll::shared_library libsnopt7_c(path_to_lib);
        // We then load the symbols we need for the SNOPT7 plugin
        snInit = boost::dll::import_symbol<void(snProblem *, char *, char *,
                                                int)>( // type of the function to import
            libsnopt7_c,                               // the library
            "snInit"                                   // name of the function to import
        );

        setIntParameter = boost::dll::import_symbol<int(snProblem *, char[], int)>( // type of the function to import
            libsnopt7_c,                                                            // the library
            "setIntParameter"                                                       // name of the function to import
        );

        setRealParameter
            = boost::dll::import_symbol<int(snProblem *, char[], double)>( // type of the function to import
                libsnopt7_c,                                               // the library
                "setRealParameter"                                         // name of the function to import
            );

        deleteSNOPT = boost::dll::import_symbol<void(snProblem *)>( // type of the function to import
            libsnopt7_c,                                            // the library
            "deleteSNOPT"                                           // name of the function to import
        );

        solveA = boost::dll::import_symbol<int(snProblem *, int, int, int, double, int, snFunA, int, int *, int *,
                                               double *, int, int *, int *, double *, double *, double *, double *,
                                               double *, int *, double *, double *, int *, double *, int *, int *,
                                               double *)>( // type of the function to import
            libsnopt7_c,                                   // the library
            "solveA"                                       // name of the function to import
        );
    } catch (const std::exception &e) {
        std::string message(
            R"(
An error occurred while loading the snopt7_c library at run-time. This is typically caused by one of the following
reasons:

- The file declared to be the snopt7_c library, i.e. )"
            + m_snopt7_c_library
            + R"(, is not a shared library containing the necessary C interface symbols (is the file path really pointing to
a valid shared library?)
 - The library is found and it does contain the C interface symbols, but it needs linking to some additional libraries that are not found
at run-time.

We report the exact text of the original exception thrown:

 )" + std::string(e.what()));
        pagmo_throw(std::invalid_argument, message);
    }
    // ------------------------- END SNOPT7 PLUGIN -------------------------------------------------------------

    // We init and set up SNOPT options
    // We init the SNOPT workspace suppressing the file output. TODO: should we allow the file output?
    snProblem snopt7_problem;
    char empty_string[] = "";

    auto problem_name = detail::s_to_C(prob.get_name());

    // Here we call snInit and ensure deleteSNOPT will be called whenever the object spr is destroyed.
    detail::sn_problem_raii<snProblem> spr(&snopt7_problem, problem_name.data(), empty_string, m_screen_output, snInit,
                                           deleteSNOPT);
    // Logic for the handling of constraints tolerances. The logic is as follows:
    // - if the user provides the "Major feasibility tolerance" option, use that *unconditionally*. Otherwise,
    // - compute the minimum tolerance min_tol among those returned by  problem.c_tol(). If zero, ignore
    //   it and use the SNOPT7 default value for "Major feasibility tolerance" (1e-6). Otherwise, use min_tol as
    //   the value for "Major feasibility tolerance".
    int res = 0;
    if (prob.get_nc() && !m_numeric_opts.count("Major feasibility tolerance")) {
        const auto c_tol = prob.get_c_tol();
        assert(!c_tol.empty());
        const double min_tol = *std::min_element(c_tol.begin(), c_tol.end());
        if (min_tol > 0.) {
            auto option_name = detail::s_to_C("Major feasibility tolerance");
            res = setRealParameter(&snopt7_problem, option_name.data(), min_tol);
            assert(res == 0);
        }
    }
    // We prevent to set the "Derivative option" option as pagmo sets it according to the value of
    // prob.has_gradient()
    if (m_integer_opts.count("Derivative option")) {
        pagmo_throw(
            std::invalid_argument,
            R"(The option "Derivative option" was set by the user. In pagmo that is not allowed, as its value is automatically set according to the value returned by has_gradient() (true -> 3, false -> 0))");
    }
    res = 0;
    // We set all the other user defined options
    for (const auto &p : m_numeric_opts) {
        auto option_name = detail::s_to_C(p.first);
        double option_value(p.second);
        res = setRealParameter(&snopt7_problem, option_name.data(), option_value);
        if (res > 0) {
            pagmo_throw(std::invalid_argument,
                        "The option '" + p.first + "' was requested by the user to be set to the float value "
                            + std::to_string(option_value)
                            + ", but SNOPT7 interface returned an error. Did you mispell the option name?");
        }
    }
    for (const auto &p : m_integer_opts) {
        auto option_name = detail::s_to_C(p.first);
        int option_value(p.second);
        res = setIntParameter(&snopt7_problem, option_name.data(), option_value);
        if (res > 0) {
            pagmo_throw(std::invalid_argument,
                        "The option '" + p.first + "' was requested by the user to be set to the int value "
                            + std::to_string(option_value)
                            + ", but SNOPT7 interface returned an error. Did you mispell the option name?");
        }
    }

    // ------- We define various inputs to call the snOptA interface
    int Cold = 0;            // Cold start
    auto nF = prob.get_nf(); // Fitness dimension
    auto n = prob.get_nx();  // Decision vector dimension

    // ------- Setting the bounds. -----------------------------------------------------------------------------
    pagmo::vector_double xlow(n), xupp(n);
    pagmo::vector_double Flow(nF), Fupp(nF);
    // decision vector.
    for (decltype(dim) i = 0u; i < dim; ++i) {
        xlow[i] = lb[i];
        xupp[i] = ub[i];
    }
    // fitness vector.
    Flow[0] = -std::numeric_limits<double>::max(); // obj
    Fupp[0] = std::numeric_limits<double>::max();
    for (decltype(prob.get_nec()) i = 0u; i < prob.get_nec(); ++i) { // ec
        Flow[i + 1] = 0.;
        Fupp[i + 1] = 0.;
    }
    for (decltype(prob.get_nic()) i = 0u; i < prob.get_nic(); ++i) { // ic
        Flow[i + 1 + prob.get_nec()] = -std::numeric_limits<double>::max();
        Fupp[i + 1 + prob.get_nec()] = 0.;
    }

    // ------- Setting the initial point ---------------------------------------------------------------------
    // We init the starting point using the inherited methods from not_population_based
    auto sel_xf = select_individual(pop);
    pagmo::vector_double x0(std::move(sel_xf.first)), fit0(std::move(sel_xf.second));
    // Initialize states, x and multipliers
    std::vector<int> xstate(n), Fstate(nF);
    pagmo::vector_double x(n), xmul(n), F(nF), Fmul(nF);
    for (decltype(x0.size()) i = 0u; i < x0.size(); i++) {
        xstate[i] = 0;
        x[i] = x0[i];
        xmul[i] = 0.;
    }
    for (decltype(x0.size()) i = 0u; i < fit0.size(); i++) {
        Fstate[i] = 0;
        F[i] = fit0[0];
        Fmul[i] = 0;
    }

    // ------- Some inits for quantities needed by the snOptA interface
    int ObjRow = 0;
    double ObjAdd = 0;
    int nS, nInf;
    double sInf;
    // We use the user workspace (iu variable) to hide a pointer to user_data,
    // so that it may be accessed in the user-defined function.
    detail::user_data info;
    info.m_prob = prob;
    info.m_verbosity = m_verbosity;
    info.m_dv = pagmo::vector_double(dim);
    snopt7_problem.iu = reinterpret_cast<int *>(&info);

    // -------- Linear Part Of the Problem. As pagmo does not support linear problems we do not use this -------
    int neA = 0;        // We switch off the linear part of the fitness
    unsigned lenA = 1u; // Thats the minimum length allowed
    std::vector<int> iAfun(lenA);
    std::vector<int> jAvar(lenA);
    pagmo::vector_double A(lenA);

    // -------- Non Linear Part Of the Problem. ----------------------------------------------------------------
    auto sparsity = prob.gradient_sparsity();
    int neG = static_cast<int>(sparsity.size());
    auto lenG = sparsity.size();
    std::vector<int> iGfun(lenG);
    std::vector<int> jGvar(lenG);
    for (decltype(sparsity.size()) i = 0u; i < sparsity.size(); ++i) {
        iGfun[i] = static_cast<int>(sparsity[i].first);
        jGvar[i] = static_cast<int>(sparsity[i].second);
    }
    if (prob.has_gradient()) {
        res = setIntParameter(&snopt7_problem, &std::string("Derivative option")[0], 3);
        assert(res == 0);
    } else {
        res = setIntParameter(&snopt7_problem, &std::string("Derivative option")[0], 0);
        assert(res == 0);
    }

    // ------- We call the snOptA interface.
    if (m_verbosity > 0u) {
        pagmo::print("SNOPT7 plugin for pagmo/pygmo: \n");
        if (prob.has_gradient_sparsity()) {
            pagmo::print("The gradient sparsity is provided by the user: ", neG, " components detected.\n");
        } else {
            pagmo::print("The gradient sparsity is assumed dense: ", neG, " components detected.\n");
        }
        if (prob.has_gradient()) {
            pagmo::print("The gradient is provided by the user.\n");
        } else {
            pagmo::print("The gradient is computed numerically by SNOPT7.\n");
        }
    }
    m_last_opt_res = solveA(&snopt7_problem, Cold, static_cast<int>(nF), static_cast<int>(n), ObjAdd, ObjRow,
                            detail::snopt_fitness_wrapper, neA, iAfun.data(), jAvar.data(), A.data(), neG, iGfun.data(),
                            jGvar.data(), xlow.data(), xupp.data(), Flow.data(), Fupp.data(), x.data(), xstate.data(),
                            xmul.data(), F.data(), Fstate.data(), Fmul.data(), &nS, &nInf, &sInf);

    if (m_verbosity > 0u) {
        pagmo::print("\n", detail::results.at(m_last_opt_res), "\n");
    }
    // ------- We reinsert the solution if better -----------------------------------------------------------
    // Store the new individual into the population, but only if it is improved.
    if (pagmo::compare_fc(F, fit0, prob.get_nec(), prob.get_c_tol())) {
        replace_individual(pop, x, F);
    }
    // ------- Store the log --------------------------------------------------------------------------------
    m_log = std::move(info.m_log);
    // ------- Handle any exception that might have been thrown during the evolve call. ---------------------
    if (info.m_eptr) {
        std::rethrow_exception(info.m_eptr);
    }
    return pop;
}

} // namespace ppnf

PAGMO_S11N_ALGORITHM_IMPLEMENT(ppnf::snopt7)