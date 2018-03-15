#ifndef PAGMO_WORHP_HPP
#define PAGMO_WORHP_HPP

#include <algorithm> // std::min_element, std::sort, std::remove_if
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem.hpp>
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
#include <pagmo/utils/constrained.hpp>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits> // std::false_type
#include <unordered_map>
#include <vector>

extern "C" {
#include "bogus_libs/worhp_lib/worhp.h"
}

namespace pagmo
{

namespace detail
{

// Usual trick with global read-only data useful to the WORHP wrapper.
template <typename = void>
struct worhp_statics {
    using mutex_t = std::mutex;
    static mutex_t library_load_mutex;
};

// Init of the statics data
template <typename T>
typename worhp_statics<T>::mutex_t worhp_statics<T>::library_load_mutex;

} // end of namespace detail

/// WORHP - (We Optimize Really Huge Problems)
/**
 * \image html worhp.png
 *
 * This class is a user-defined algorithm (UDA) that contains a plugin to the WORHP (We Optimize Really Huge Problems)
 * solver, a software package for large-scale nonlinear optimization. WORHP is a powerful solver that is able to handle
 * robustly and efficiently constrained nonlinear opimization problems also at high dimensionalities. The wrapper
 * was developed around the version 1.12 of WORHP and the Full Feature Interface (FFI) useing the Unified Solver
 * Interface and the Reverse Communication paradigm (see worhp user manual).
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. warning::
 *
 *    Unfortunately, the WORHP library is only available acquiring a licence. You can consult the web pages at
 * (https://worhp.de/) for further information. There you will be able to download the correct library for your
 * architecture and obtain a license file. You will be able to specify the location of the downloaded library when
 * constructing this UDA.
 *
 * \endverbatim
 *
 *
 * Worhp is designed to efficiently solve small- to large-scale constrained optimisation problems, where
 * the objective function and the constraints are sufficiently smooth, and may be linear, quadratic or nonlinear. It is
 * designed to find locally optimal points of optimisation problems, which may be globally optimal, depending on the
 * problem structure, the initial guess and other factors. Worhp combines  a  Sequential  Quadratic  Programming  (SQP)
 * method  on  the general nonlinear level with a primal-dual Interior Point (IP) method on the quadratic subproblem
 * level, to generate a sequence of search directions, which are subject to line search using the Augmented Lagrangian
 * or L1 merit function.
 *
 * Worhp needs first and second order derivatives, which can be supplied by the user, or approximated by finite
 * differences or quasi-Newton methods.
 *
 * In order to support pagmo's population-based optimisation model, worhp::evolve() will select
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
 *    We developed this plugin for the WORHP version 1.12, but it will also work woth different versions of the library
 * as far as the API has not changed and the following symbols are found:
 *
 * .. warning::
 *
 *    A moved-from :cpp:class:`worhp::snopt7` is destructible and assignable. Any other operation will result
 *    in undefined behaviour.
 *
 * .. warning::
 *
 *    The possibility to exploit the linear part of the problem fitness, part of the original WORHP library,
 *    is deactivated in this plugin for pagmo.
 *
 * .. seealso::
 *
 *    https://worhp.de/
 *
 *
 * \endverbatim
 */
class worhp : public not_population_based
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
    // using log_line_type = std::tuple<unsigned long, double, vector_double::size_type, double, bool>;
    /// Log type.
    /**
     * The algorithm log is a collection of snopt7::log_line_type data lines, stored in chronological order
     * during the optimisation if the verbosity of the algorithm is set to a nonzero value
     * (see snopt7::set_verbosity()).
     */
    // using log_type = std::vector<log_line_type>;

    ///  Constructor.
    /**
     * The algorithm WORHP can be constructed in two different ways. According to the user
     * choice, only one among the original WORHP screen output and the pagmo logging system will
     * be activated.
     *
     * @param screen_output when ``true`` will activate the screen output from the WORHP library, otherwise
     * will let pagmo regulate logs and screen_output via its pagmo::algorithm::set_verbosity mechanism.
     * @param worhp_library The filename, including the absolute path, of the worhp library.
     *
     */
    worhp(bool screen_output = false, std::string worhp_library = "/usr/local/lib/libworhp.so")
        : m_worhp_library(worhp_library), m_screen_output(screen_output){};

    /// Evolve population.
    /**
     * This method will select an individual from \p pop, optimise it using the WORHP USI interface, replace an
     * individual in \p pop with the optimised individual, and finally return \p pop. The individual selection and
     * replacement criteria can be set via set_selection(const std::string &), set_selection(population::size_type),
     * set_replacement(const std::string &) and set_replacement(population::size_type). The WORHP solver will then run
     * until one of the stopping criteria is satisfied, and the return status of the WORHP solver will be recorded (it
     * can be fetched with get_last_opt_result()).
     *
     * \verbatim embed:rst:leading-asterisk
     *
     * .. warning::
     *
     *    All options passed to the snOptA interface are those set by the user via the pagmo::snopt7 interface, or
     *    where no user specifications are available, to the default detailed on the User Manual available online but
     *    with the following exception: "Major feasibility tolerance" is set to the default value 1E-6 or to the minimum
     *    among the values returned by pagmo::problem::get_c_tol() if not zero.
     *
     *
     * \endverbatim
     *
     * @param pop the population to be optimised.
     *
     * @return the optimised population.
     *
     * @throws std::invalid_argument in the following cases:
     * - the population's problem is multi-objective or stochastic
     * - the population is empty.
     * @throws unspecified any exception thrown by the public interface of pagmo::problem or
     * pagmo::not_population_based.
     */
    population evolve(population pop) const
    {
        // We store some useful properties
        const auto &prob
            = pop.get_problem(); // This is a const reference, so using set_seed, for example, will not work
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
            pagmo_throw(std::invalid_argument, get_name() + " does not work on an empty population");
        }
        // ---------------------------------------------------------------------------------------------------------

        // ------------------------- WORHP PLUGIN (we attempt loading the worhp library at run-time)--------------
        // We first declare the prototypes of the functions used from the library
        std::function<void(int *, char *, Params *)> ReadParams;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpPreInit;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpInit;
        std::function<bool(const Control *, int)> GetUserAction;
        std::function<bool(Control *, int)> DoneUserAction;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> IterationOutput;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> Worhp;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> StatusMsg;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpFree;
        std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpFidif;
        std::function<void(worhp_print_t)> SetWorhpPrint;

        // We then try to load the library at run time and locate the symbols used.
        try {
            // Here we import at runtime the worhp library and protect the whole try block with a mutex
            std::lock_guard<std::mutex> lock(detail::worhp_statics<>::library_load_mutex);
            boost::filesystem::path path_to_lib(m_worhp_library);
            if (!boost::filesystem::is_regular_file(path_to_lib)) {
                pagmo_throw(std::invalid_argument, "The worhp library path was constructed to be: "
                                                       + path_to_lib.string() + " and it does not appear to be a file");
            }
            boost::dll::shared_library libworhp(path_to_lib);
            // We then load the symbols we need for the WORHP plugin
            WorhpPreInit = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                                   Control *)>( // type of the function to import
                libworhp,                                       // the library
                "WorhpPreInit"                                  // name of the function to import
            );
            WorhpInit = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                                Control *)>( // type of the function to import
                libworhp,                                    // the library
                "WorhpInit"                                  // name of the function to import
            );
            ReadParams = boost::dll::import<void(int *, char *, Params *)>( // type of the function to import
                libworhp,                                                   // the library
                "ReadParams"                                                // name of the function to import
            );
            SetWorhpPrint = boost::dll::import<void(worhp_print_t)>( // type of the function to import
                libworhp,                                            // the library
                "SetWorhpPrint"                                      // name of the function to import
            );
            GetUserAction = boost::dll::import<bool(const Control *, int)>( // type of the function to import
                libworhp,                                                   // the library
                "GetUserAction"                                             // name of the function to import
            );
            DoneUserAction = boost::dll::import<bool(Control *, int)>( // type of the function to import
                libworhp,                                              // the library
                "DoneUserAction"                                       // name of the function to import
            );
            IterationOutput = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                                      Control *)>( // type of the function to import
                libworhp,                                          // the library
                "IterationOutput"                                  // name of the function to import
            );
            Worhp = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                            Control *)>( // type of the function to import
                libworhp,                                // the library
                "Worhp"                                  // name of the function to import
            );
            StatusMsg = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                                Control *)>( // type of the function to import
                libworhp,                                    // the library
                "StatusMsg"                                  // name of the function to import
            );
            WorhpFree = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                                Control *)>( // type of the function to import
                libworhp,                                    // the library
                "WorhpFree"                                  // name of the function to import
            );
            WorhpFidif = boost::dll::import<void(OptVar *, Workspace *, Params *,
                                                 Control *)>( // type of the function to import
                libworhp,                                     // the library
                "WorhpFidif"                                  // name of the function to import
            );
        } catch (const std::exception &e) {
            std::string message(
                R"(
An error occurred while loading the worhp library at run-time. This is typically caused by one of the following
reasons:

- The file declared to be the worhp library, i.e. )"
                + m_worhp_library
                + R"(, is not found or is found but it is not a shared library containing the necessary symbols 
(is the file path really pointing to a valid shared library?)
 - The library is found and it does contain the symbols, but it needs linking to some additional libraries that are not found
at run-time.

We report the exact text of the original exception thrown:

 )" + std::string(e.what()));
            pagmo_throw(std::invalid_argument, message);
        }
        // ------------------------- END WORHP PLUGIN -------------------------------------------------------------

        // With reference to the worhp User Manual (V1.12)
        // USI-0:  Call WorhpPreInit to properly initialise the (empty) data structures.
        OptVar opt;
        Workspace wsp;
        Params par;
        Control cnt;
        WorhpPreInit(&opt, &wsp, &par, &cnt);

        // USI-1: Read parameters from XML
        // Note that a file named "param.xml" will be searched in the current directory only if the environment variable
        // WORHP_PARAM_FILE is not set. Otherwise the WORHP_PARAM_FILE will be used. The number of parameterts that are
        // not getting default values will be stored in n_xml_param
        int n_xml_param;
        ReadParams(&n_xml_param, const_cast<char *>("param.xml"), &par);

        // USI-2: Specify problem dimensions
        opt.n = dim;
        opt.m = prob.get_nc(); // number of constraints
        auto n_eq = prob.get_nec();
        // Get the sparsity pattern of the gradient
        auto pagmo_gs = prob.gradient_sparsity();
        // Determine where the gradients of the constraints start.
        const auto it = std::lower_bound(pagmo_gs.begin(), pagmo_gs.end(), sparsity_pattern::value_type(1u, 0u));
        // Split the sparsity into f and g parts
        sparsity_pattern fs(pagmo_gs.begin(), it);
        sparsity_pattern gs(it, pagmo_gs.end());

        // NOTE: Worhp requires a single sparsity pattern for the hessian of the lagrangian (that is,
        // the pattern must be valid for objfun and all constraints), but we provide a separate sparsity pattern for
        // objfun and every constraint. We will thus need to merge our sparsity patterns in a single sparsity
        // pattern.
        sparsity_pattern merged_hs;
        if (prob.has_hessians_sparsity()) {
            // Store the original hessians sparsity only if it is user-provided.
            auto hs = prob.hessians_sparsity();
            for (const auto &sp : hs) {
                // NOTE: we need to create a separate copy each time as std::set_union() requires distinct ranges.
                const auto old_merged_hs(merged_hs);
                merged_hs.clear();
                std::set_union(old_merged_hs.begin(), old_merged_hs.end(), sp.begin(), sp.end(),
                               std::back_inserter(merged_hs));
            }
        } else {
            // If the hessians sparsity is not user-provided, dense patterns are assumed.
            merged_hs = detail::dense_hessian(prob.get_nx());
        }
        wsp.DF.nnz = fs.size();
        wsp.DG.nnz = gs.size();
        wsp.HM.nnz = merged_hs.size();

        // This flag informs Worhp that f and g cannot be evaluated seperately (TODO: remove if pagmo will implement a
        // caching system)
        par.FGtogether = true;

        // We deal with the gradient
        if (prob.has_gradient()) {
            par.UserDF
                = true; // TODO: MAKE
                        // true!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!erty54e654634b67
                        // 67bn567 756b7n7567g77f674vb76bv
            par.UserDG = true;
        } else {
            par.UserDF = false;
            par.UserDG = false;
        }
        if (prob.has_hessians()) {
            par.UserHM
                = false; // TODO: MAKE
                         // true!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!erty54e654634b67
                         // 67bn567 756b7n7567g77f674vb76bv
        } else {
            par.UserHM = false;
        }

        // USI-3: Allocate solver memory
        WorhpInit(&opt, &wsp, &par, &cnt);

        // USI-5: Set initial values and deal with gradients / hessians
        // We define the initial value for the chromosome
        // We init the starting point using the inherited methods from not_population_based
        auto sel_xf = select_individual(pop);
        vector_double x0(std::move(sel_xf.first)), f0(std::move(sel_xf.second)); // TODO: is f0 useful?
        for (decltype(opt.n) i = 0u; i < opt.n; ++i) {
            opt.X[i] = x0[i];
        }
        opt.F = wsp.ScaleObj * f0[0];
        for (decltype(opt.m) i = 0; i < opt.m; ++i) {
            opt.G[i] = f0[i + 1];
        }

        // USI-6: Set the constraint bounds
        // Box bounds
        for (decltype(opt.n) i = 0; i < opt.n; ++i) {
            opt.Lambda[i] = 0;
            opt.XL[i] = lb[i];
            opt.XU[i] = ub[i];
        }
        // Equality constraints
        for (decltype(n_eq) i = 0u; i < n_eq; ++i) {
            opt.Mu[i] = 0;
            opt.GL[i] = 0;
            opt.GU[i] = 0;
        }
        // Inequality constraints
        for (decltype(opt.m) i = n_eq; i < opt.m; ++i) {
            opt.Mu[i] = 0;
            opt.GL[i] = -par.Infty;
            opt.GU[i] = 0;
        }

        /*
         * Specify matrix structures in CS format, using Fortran indexing,
         * i.e. 1...N instead of 0...N-1, to describe the matrix structure.
         */

        // -------------------------------------------------------------------------------------------------------------------------
        // Assign sparsity structure to DF
        if (wsp.DF.NeedStructure) {
            for (decltype(fs.size()) i = 0; i < fs.size(); ++i) {
                // NOTE: the +1 is because of fortran notation is required by WORHP (maledetti).
                wsp.DF.row[i] = fs[i].second + 1;
            }
        }

        // -------------------------------------------------------------------------------------------------------------------------
        // Create the corresponding index map between pagmo and worhp sparse representation of the gradient
        std::vector<vector_double::size_type> gs_idx_map(gs.size());
        std::iota(gs_idx_map.begin(), gs_idx_map.end(), 0);
        std::sort(gs_idx_map.begin(), gs_idx_map.end(),
                  [&gs](std::vector<vector_double::size_type>::size_type &idx1,
                        std::vector<vector_double::size_type>::size_type &idx2) -> bool {
                      return (gs[idx1].second < gs[idx2].second
                              || (!(gs[idx2].second < gs[idx1].second) && gs[idx1].first < gs[idx2].first));
                  });
        // Assign sparsity structure to DG if not dense.
        if (wsp.DG.NeedStructure) {
            for (decltype(gs_idx_map.size()) i = 0u; i < gs_idx_map.size(); ++i) {
                // NOTE: no need for +1 here as in pagmo 0 is the objfun already stripped from here.
                wsp.DG.row[i] = gs[gs_idx_map[i]].first;
                // NOTE: the +1 is because of fortran notation is required by WORHP (maledetti).
                wsp.DG.col[i] = gs[gs_idx_map[i]].second + 1;
            }
        }

        // -------------------------------------------------------------------------------------------------------------------------
        // Create the corresponding index map between pagmo and worhp sparse representation of the lower triangular part
        // of the hessian of the lagrangian
        std::vector<vector_double::size_type> hs_idx_map(merged_hs.size());
        std::iota(hs_idx_map.begin(), hs_idx_map.end(), 0);
        // We remove the diagonal entries from the hessian sparsity as merged from pagmo (if present)
        auto it2 = std::remove_if(hs_idx_map.begin(), hs_idx_map.end(),
                                  [merged_hs](std::vector<vector_double::size_type>::size_type &idx) -> bool {
                                      return (merged_hs[idx].first == merged_hs[idx].second);
                                  });
        hs_idx_map.resize(it2 - hs_idx_map.begin());

        // Assign sparsity structure to HM
        if (wsp.HM.NeedStructure) {
            /*
             * In WORHP the HM sparsity requires lower triangular entries first,
             * then all the diagonal elements (also the zeros) (cani maledetti^2)
             */
            // We remove the diagonal entries from the hessian sparsity as merged from pagmo (if present)
            auto it3 = std::remove_if(merged_hs.begin(), merged_hs.end(),
                                      [](std::pair<vector_double::size_type, vector_double::size_type> &x) -> bool {
                                          return (x.first == x.second);
                                      });
            merged_hs.resize(it3 - merged_hs.begin());

            // Sort the resulting hessian of the lagrangian sparsity according to worhp twisted choice.
            // Lexicographic from right to left, i.e. ((1,0),(2,0),(0,1),(1,1), )
            std::sort(merged_hs.begin(), merged_hs.end(),
                      [](std::pair<vector_double::size_type, vector_double::size_type> &x,
                         std::pair<vector_double::size_type, vector_double::size_type> &y) -> bool {
                          return (x.second < y.second || (!(y.second < x.second) && x.first < y.first));
                      });

            // Strict lower triangle
            for (decltype(merged_hs.size()) i = 0u; i < merged_hs.size(); ++i) {
                wsp.HM.row[i] = merged_hs[i].first
                                + 1; // NOTE: the +1 is because of fortran notation is required by WORHP (maledetti).
                wsp.HM.col[i] = merged_hs[i].second
                                + 1; // NOTE: the +1 is because of fortran notation is required by WORHP (maledetti).
            }

            // Diagonal
            for (decltype(dim) i = 0; i < dim; ++i) {
                wsp.HM.row[merged_hs.size() + i] = i + 1;
                wsp.HM.col[merged_hs.size() + i] = i + 1;
            }
        }

        // -------------------------------------------------------------------------------------------------------------------------

        // USI-7: Run the solver
        /*
         * WORHP Reverse Communication loop.
         * In every iteration poll GetUserAction for the requested action, i.e. one
         * of {callWorhp, iterOutput, evalF, evalG, evalDF, evalDG, evalHM, fidif}.
         *
         * Make sure to reset the requested user action afterwards by calling
         * DoneUserAction, except for 'callWorhp' and 'fidif'.
         */
        while (cnt.status < TerminateSuccess && cnt.status > TerminateError) {
            /*
             * WORHP's main routine.
             * Do not manually reset callWorhp, this is only done by the FD routines.
             */
            if (GetUserAction(&cnt, callWorhp)) {
                Worhp(&opt, &wsp, &par, &cnt);
                // No DoneUserAction!
            }

            /*
             * Show iteration output.
             * The call to IterationOutput() may be replaced by user-defined code.
             */
            if (GetUserAction(&cnt, iterOutput)) {
                IterationOutput(&opt, &wsp, &par, &cnt);
                DoneUserAction(&cnt, iterOutput);
            }

            /*
             * Evaluate the objective function.
             * The call to UserF may be replaced by user-defined code.
             */
            if (GetUserAction(&cnt, evalF)) {
                UserF(&opt, &wsp, &par, &cnt, pop);
                DoneUserAction(&cnt, evalF);
            }

            /*
             * Evaluate the constraints.
             * The call to UserG may be replaced by user-defined code.
             */
            if (GetUserAction(&cnt, evalG)) {
                UserG(&opt, &wsp, &par, &cnt, pop);
                DoneUserAction(&cnt, evalG);
            }

            /*
             * Evaluate the gradient of the objective function.
             * The call to UserDF may be replaced by user-defined code.
             */
            if (GetUserAction(&cnt, evalDF)) {
                UserDF(&opt, &wsp, &par, &cnt, pop);
                DoneUserAction(&cnt, evalDF);
            }

            /*
             * Evaluate the Jacobian of the constraints.
             * The call to UserDG may be replaced by user-defined code.
             */
            if (GetUserAction(&cnt, evalDG)) {
                UserDG(&opt, &wsp, &par, &cnt, pop, gs_idx_map);
                DoneUserAction(&cnt, evalDG);
            }

            /*
             * Use finite differences with RC to determine derivatives
             * Do not reset fidif, this is done by the FD routine.
             */
            if (GetUserAction(&cnt, fidif)) {
                WorhpFidif(&opt, &wsp, &par, &cnt);
                // No DoneUserAction!
            }
        }

        // ------- We reinsert the solution if better -----------------------------------------------------------
        // Store the new individual into the population, but only if it is improved.
        vector_double x_final(dim, 0);
        vector_double f_final(prob.get_nf(), 0);
        for (int i = 0; i < opt.n; ++i) {
            x_final[i] = opt.X[i];
        }
        f_final = prob.fitness(x_final);

        if (compare_fc(f_final, f0, prob.get_nec(), prob.get_c_tol())) {
            replace_individual(pop, x_final, f_final);
        }

        StatusMsg(&opt, &wsp, &par, &cnt);
        WorhpFree(&opt, &wsp, &par, &cnt);

        return pop;
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
     *    object pagmo::snopt7 passing ``True`` as argument. In this case verbosity will not be allowed to be set.
     *
     * \endverbatim
     *
     */
    void set_verbosity(unsigned n)
    {
        if (m_screen_output && n != 0u) {
            pagmo_throw(std::invalid_argument,
                        "Cannot set verbosity to a >0 value if WORHP screen output is choosen upon construction.");
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
    // const log_type &get_log() const
    //{
    //    return m_log;
    //}
    /// Gets the verbosity level
    /**
     * @return the verbosity level
     */
    unsigned int get_verbosity() const
    {
        return m_verbosity;
    }
    /// Algorithm name
    /**
     * One of the optional methods of any user-defined algorithm (UDA).
     *
     * @return a string containing the algorithm name
     */
    std::string get_name() const
    {
        return "WORHP";
    }
    /// Get extra information about the algorithm.
    /**
     * @return a human-readable string containing useful information about the algorithm's properties
     * (e.g., the SNOPT7 user-set options, the selection/replacement policies, etc.), the snopt7_c library path
     */
    std::string get_extra_info() const
    {
        std::ostringstream ss;
        stream(ss, "\tWorhp library filename: ", m_worhp_library);
        if (!m_screen_output) {
            stream(ss, "\n\tScreen output: (pagmo/pygmo) - verbosity ", std::to_string(m_verbosity));
        } else {
            stream(ss, "\n\tScreen output: (worhp)");
        }
        stream(ss, "\n\tIndividual selection ");
        if (boost::any_cast<population::size_type>(&m_select)) {
            stream(ss, "idx: ", std::to_string(boost::any_cast<population::size_type>(m_select)));
        } else {
            stream(ss, "policy: ", boost::any_cast<std::string>(m_select));
        }
        stream(ss, "\n\tIndividual replacement ");
        if (boost::any_cast<population::size_type>(&m_replace)) {
            stream(ss, "idx: ", std::to_string(boost::any_cast<population::size_type>(m_replace)));
        } else {
            stream(ss, "policy: ", boost::any_cast<std::string>(m_replace));
        }
        stream(ss, "\n");
        return ss.str();
    }
    /// Object serialization
    /**
     * This method will save/load \p this into the archive \p ar.
     *
     * @param ar target archive.
     *
     * @throws unspecified any exception thrown by the serialization of the UDA and of primitive types.
     */
    template <typename Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::base_class<not_population_based>(this), m_worhp_library);
    }

private:
    // Used to suppress screen output from worhp
    static void no_screen_output(int, const char[]){};
    // Objective function
    void UserF(OptVar *opt, Workspace *wsp, Params *par, Control *cnt, const population &pop) const
    {
        double *X = opt->X; // Abbreviate notation
        const auto &prob = pop.get_problem();
        auto dim = prob.get_nx();
        vector_double x(X, X + dim);
        auto f = prob.fitness(x);
        opt->F = wsp->ScaleObj * f[0];
    }
    // Constraints
    void UserG(OptVar *opt, Workspace *wsp, Params *par, Control *cnt, const population &pop) const
    {
        double *X = opt->X; // Abbreviate notation
        const auto &prob = pop.get_problem();
        auto dim = prob.get_nx();
        vector_double x(X, X + dim);
        auto f = prob.fitness(x);
        for (decltype(prob.get_nc()) i = 0; i < prob.get_nc(); ++i) {
            opt->G[i] = f[i + 1];
        }
    }
    // Gradient for the objective function
    void UserDF(OptVar *opt, Workspace *wsp, Params *par, Control *cnt, const population &pop) const
    {
        const auto &prob = pop.get_problem();
        auto dim = prob.get_nx();
        vector_double x(opt->X, opt->X + dim);
        auto g = prob.gradient(x);
        for (decltype(wsp->DF.nnz) i = 0; i < wsp->DF.nnz; ++i) {
            wsp->DF.val[i] = g[i];
        }
    }

    // Gradient for the constraints
    void UserDG(OptVar *opt, Workspace *wsp, Params *par, Control *cnt, const population &pop,
                const std::vector<vector_double::size_type> &gs_idx_map) const
    {
        const auto &prob = pop.get_problem();
        auto dim = prob.get_nx();
        vector_double x(opt->X, opt->X + dim);
        auto g = prob.gradient(x);
        for (decltype(wsp->DG.nnz) i = 0; i < wsp->DG.nnz; ++i) {
            wsp->DG.val[i] = g[wsp->DF.nnz + gs_idx_map[i]];
        }
    }

    // The absolute path to the worhp library
    std::string m_worhp_library;

    // Activates the original worhp screen output
    bool m_screen_output;
    unsigned int m_verbosity;

    // Deleting the methods load save public in base as to avoid conflict with serialize
    template <typename Archive>
    void load(Archive &ar) = delete;
    template <typename Archive>
    void save(Archive &ar) const = delete;
};

} // namespace pagmo

PAGMO_REGISTER_ALGORITHM(pagmo::worhp)

#endif // PAGMO_SNOPT7
