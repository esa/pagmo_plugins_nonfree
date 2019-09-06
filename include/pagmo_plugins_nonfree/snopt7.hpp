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

#ifndef PAGMO_SNOPT7_HPP
#define PAGMO_SNOPT7_HPP

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

extern "C" {
#include "bogus_libs/snopt7_c_lib/snopt7_c.h"
}

// The following lines are a workaround for the boost::is_object limit of 24 maximum arguments. When called with
// more arguments boost::is_object actually fails to detect correctly if T is an object. To workaround this
// we provide a specialization to the template class that works exactly on our used signature.
namespace boost
{
template <>
struct is_object<int(snProblem_76 *, int, int, int, double, int, snFunA, int, int *, int *, double *, int, int *, int *,
                     double *, double *, double *, double *, double *, int *, double *, double *, int *, double *,
                     int *, int *, double *)> : std::false_type {
};
template <>
struct is_object<int(snProblem_77 *, int, int, int, double, int, snFunA, int, int *, int *, double *, int, int *, int *,
                     double *, double *, double *, double *, double *, int *, double *, double *, int *, double *,
                     int *, int *, double *)> : std::false_type {
};
} // namespace boost

namespace pagmo
{

namespace detail
{
// Encapsulating struct for data that are used in the fitness wrapper.
struct user_data {
    // Single entry of the log (objevals, objval, n of unsatisfied const, constr. violation, feasibility).
    using log_line_type = std::tuple<unsigned long, double, vector_double::size_type, double, bool>;
    // The log.
    using log_type = std::vector<log_line_type>;
    // The problem stored in the evolve() population
    problem m_prob;
    // A preallocated decision vector
    vector_double m_dv;
    // The verbosity
    unsigned m_verbosity;
    // The log
    log_type m_log;
    // A counter
    unsigned long m_objfun_counter = 0;
    // This exception pointer will be null, unless
    // an error is raised during the computation of the objfun
    // or constraints. If not null, it will be re-thrown
    // in the evolve() method.
    std::exception_ptr m_eptr;
};

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

// Usual trick with global read-only data useful to the SNOPT7 wrapper.
template <typename = void>
struct snopt_statics {
    // A map to link a human-readable description to snOptA return codes for the
    // call result.
    using result_map_t = std::unordered_map<int, std::string>;
    static const result_map_t results;
    using mutex_t = std::mutex;
    static mutex_t library_load_mutex;
};

// Init of the statics data
template <typename T>
typename snopt_statics<T>::mutex_t snopt_statics<T>::library_load_mutex;

template <typename T>
const typename snopt_statics<T>::result_map_t snopt_statics<T>::results
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

extern "C" {

// Wrapper to connect pagmo's fitness calculation machinery to SNOPT7's.
// NOTE: this function needs to be passed to the SNOPT7 C API, and as such it needs to be
// declared within an 'extern "C"' block (otherwise, it might be UB to pass C++ function pointers
// to a C API).
// https://www.reddit.com/r/cpp/comments/4fqfy7/using_c11_capturing_lambdas_w_vanilla_c_api/d2b9bh0/
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
                    = detail::test_eq_constraints(fit.data() + 1, fit.data() + 1 + p.get_nec(), ctol.data());
                const auto c1ineq = detail::test_ineq_constraints(fit.data() + 1 + p.get_nec(), fit.data() + fit.size(),
                                                                  ctol.data() + p.get_nec());
                // This will be the total number of violated constraints.
                const auto nv = p.get_nc() - c1eq.first - c1ineq.first;
                // This will be the norm of the violation.
                const auto l = c1eq.second + c1ineq.second;
                // Test feasibility.
                const auto feas = p.feasibility_f(fit);

                if (!(f_count / verb % 50u)) {
                    // Every 50 lines print the column names.
                    print("\n", std::setw(10), "objevals:", std::setw(15), "objval:", std::setw(15),
                          "violated:", std::setw(15), "viol. norm:", '\n');
                }
                // Print to screen the log line.
                print(std::setw(10), f_count + 1u, std::setw(15), fit[0], std::setw(15), nv, std::setw(15), l,
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
} // extern "C"
} // namespace detail

/// SNOPT 7 - (Sparse Nonlinear OPTimizer, Version 7)
/**
 * \image html sol.png
 *
 * This class is a user-defined algorithm (UDA) that contains a plugin to the Sparse Nonlinear OPTimizer (SNOPT)
 * solver, a software package for large-scale nonlinear optimization. SNOPT is a powerful solver that is able to handle
 * robustly and efficiently constrained nonlinear opimization problems also at high dimensionalities. Since the wrapper
 * was developed around the version 7 of SNOPT the class is called pagmo::snopt7.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. note::
 *
 *    SNOPT7 fortran code is only available acquiring a licence.
 *    If you do have such a licence, then you will also have the fortran files and can build them into the library
 *    snopt7 (one single library). The library snopt7_c will then need to be built,
 *    compiling the correct release of the project https://github.com/snopt/snopt-interface. The library thus created
 *    will link to your fortran snopt7 library. As an alternative you may have only one library libsnopt7 containing
 *    both the Fortran and the C interface (this is the case, for example, of the library you can download for
 *    evaluation).
 *
 * \endverbatim
 *
 *
 * SNOPT7 supports only single-objective minimisation, using a sequential quadratic programming (SQP) algorithm.
 * Search directions are obtained from QP subproblems that minimize a quadratic model of the Lagrangian function
 * subject to linearized constraints. An augmented Lagrangian merit function is reduced along each search
 * direction to ensure convergence from any starting point.
 *
 * In order to support pagmo's population-based optimisation model, snopt7::evolve() will select
 * a single individual from the input pagmo::population to be optimised.
 * If the optimisation produces an improved individual (as established by pagmo::compare_fc()),
 * the optimised individual will be inserted back into the population.
 * The selection and replacement strategies can be configured via set_selection(const std::string &),
 * set_selection(population::size_type), set_replacement(const std::string &) and
 * set_replacement(population::size_type).
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. note::
 *
 *    This plugin was tested with snopt version 7.2 as well as with the compiled evaluation libraries (7.7)
 *    made available via the snopt7 official web site (C/Fortran library).
 *
 * .. warning::
 *
 *    Constructing this class with an inconsistent \p minor_version parameter results in undefined behaviour.
 *
 * .. warning::
 *
 *    A moved-from :cpp:class:`pagmo::snopt7` is destructible and assignable. Any other operation will result
 *    in undefined behaviour.
 *
 * .. warning::
 *
 *    The possibility to exploit the linear part of the problem fitness, part of the original SNOPT7 library,
 *    is deactivated in this plugin for pagmo.
 *
 *
 * .. seealso::
 *
 *    https://web.stanford.edu/group/SOL/snopt.htm.
 *
 * .. seealso::
 *
 *    https://web.stanford.edu/group/SOL/guides/sndoc7.pdf.
 *
 * \endverbatim
 */
class snopt7 : public not_population_based
{
public:
    /// Single data line for the algorithm's log.
    /**
     * A log data line is a tuple consisting of:
     * - the number of objective function evaluations made so far,
     * - the objective function value for the current decision vector,
     * - the number of constraints violated by the current decision vector,
     * - the constraints violation norm for the current decision vector,
     * - a boolean flag signalling the feasibility of the current decision vector.
     */
    using log_line_type = std::tuple<unsigned long, double, vector_double::size_type, double, bool>;
    /// Log type.
    /**
     * The algorithm log is a collection of snopt7::log_line_type data lines, stored in chronological order
     * during the optimisation if the verbosity of the algorithm is set to a nonzero value
     * (see snopt7::set_verbosity()).
     */
    using log_type = std::vector<log_line_type>;

private:
    static_assert(std::is_same<log_line_type, detail::user_data::log_line_type>::value, "Invalid log line type.");
    // Small helper function to convert a string to something that the C API can eat (i.e. retval.data())
    static std::vector<char> s_to_C(const std::string &);

public:
    snopt7(bool, std::string, unsigned);
    population evolve(population) const;
    void set_verbosity(unsigned);
    const log_type &get_log() const;
    unsigned int get_verbosity() const;
    std::string get_name() const;
    std::string get_extra_info() const;
    template <typename Archive>
    void serialize(Archive &ar, unsigned);
    void set_integer_option(const std::string &, int );
    void set_integer_options(const std::map<std::string, int> &);
    std::map<std::string, int> get_integer_options() const;
    void set_numeric_option(const std::string &, double );
    void set_numeric_options(const std::map<std::string, double> &);
    std::map<std::string, double> get_numeric_options() const;
    void reset_integer_options();
    void reset_numeric_options();
    int get_last_opt_result() const;
private:
    template <typename snProblem>
    population evolve_version(population &) const;

    // The absolute path to the snopt7 lib
    std::string m_snopt7_c_library;
    // Minor snopt7 version (distinguishing among 7.2 and 7.7 APIs)
    unsigned m_minor_version;
    // Options maps.
    std::map<std::string, int> m_integer_opts;
    std::map<std::string, double> m_numeric_opts;
    // Solver return status.
    mutable int m_last_opt_res = 0;
    // Activates the original snopt screen output
    bool m_screen_output;
    unsigned int m_verbosity;
    mutable log_type m_log;

    // Deleting the methods load save public in base as to avoid conflict with serialize
    template <typename Archive>
    void load(Archive &ar) = delete;
    template <typename Archive>
    void save(Archive &ar) const = delete;
};

} // namespace pagmo

PAGMO_S11N_ALGORITHM_EXPORT_KEY(pagmo::snopt7)

#endif // PAGMO_SNOPT7
