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

#include <algorithm> // std::min_element, std::sort, std::remove_if
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/map.hpp>
#include <iomanip>
#include <mutex>
#include <numeric>
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

#include "../include/pagmo_plugins_nonfree/bogus_libs/worhp_lib/worhp_bogus.h"
#include <pagmo_plugins_nonfree/worhp.hpp>

// MINGW-specific warnings.
#if defined(__GNUC__) && defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
#pragma GCC diagnostic ignored "-Wsuggest-attribute=const"
#endif

using namespace pagmo;

namespace ppnf
{
namespace detail
{
// We use this to ensure WorhpFree is called also if exceptions occur.
struct worhp_raii {
    worhp_raii(OptVar *o, Workspace *w, Params *p, Control *c,
               std::function<void(OptVar *, Workspace *, Params *, Control *)> &WorhpInit,
               std::function<void(OptVar *, Workspace *, Params *, Control *)> &WorhpFree)
        : m_o(o), m_w(w), m_p(p), m_c(c), m_WorhpFree(WorhpFree)
    {
        WorhpInit(m_o, m_w, m_p, m_c);
    }
    ~worhp_raii()
    {
        m_WorhpFree(m_o, m_w, m_p, m_c);
    }
    OptVar *m_o;
    Workspace *m_w;
    Params *m_p;
    Control *m_c;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> m_WorhpFree;
};
namespace
{
// Used to suppress screen output from worhp
void no_screen_output(int, const char[]) {}
// Mutex to protect the library loading.
std::mutex library_load_mutex;
} // namespace

} // end of namespace detail

worhp::worhp(bool screen_output, std::string worhp_library)
    : m_worhp_library(worhp_library), m_integer_opts(), m_numeric_opts(), m_bool_opts(), m_screen_output(screen_output),
      m_verbosity(0), m_log()
{
}

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
 *    All options passed to the WORHP interface are determined first by the xml parameter file, or (if not found) by
 *    the default options. Then FGtogether is set to true (for constrained problems) and UserDF, UserDG , UserHM to
 *    the values detected by the pagmo::has_gradient, pagmo::has_hessians methods. TolFeas is then set to be the
 *    minimum of prob.get_c_tol() if not 0. All the other options, contained in the data members m_integer_opts,
 *    m_numeric_opts and m_bool_opts are set after and thus overwrite the above rules.
 *
 * \endverbatim
 *
 * @param pop the population to be optimised.
 *
 * @return the optimised population.
 *
 * @throws std::invalid_argument if a version mismatch is found between the declared library and 1.12
 * @throws std::invalid_argument in the following cases:
 * - the population's problem is multi-objective or stochastic
 * @throws unspecified any exception thrown by the public interface of pagmo::problem or
 * pagmo::not_population_based.
 */
population worhp::evolve(population pop) const
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
    // ------------------------- WORHP PLUGIN (we attempt loading the worhp library at run-time)--------------
    // We first declare the prototypes of the functions used from the library
    std::function<void(int *, const char[], Params *)> ReadParams;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpPreInit;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpInit;
    std::function<bool(const Control *, int)> GetUserAction;
    std::function<void(Control *, int)> DoneUserAction;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> IterationOutput;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> Worhp;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> StatusMsg;
    std::function<void(OptVar *, Workspace *, Params *, Control *, char message[])> StatusMsgString;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpFree;
    std::function<void(OptVar *, Workspace *, Params *, Control *)> WorhpFidif;
    std::function<bool(Params *, const char *, bool)> WorhpSetBoolParam;
    std::function<bool(Params *, const char *, int)> WorhpSetIntParam;
    std::function<bool(Params *, const char *, double)> WorhpSetDoubleParam;
    std::function<void(int *major, int *minor, char patch[PATCH_STRING_LENGTH])> WorhpVersion;
    std::function<void(worhp_print_t)> SetWorhpPrint;

    boost::filesystem::path library_filename(m_worhp_library);
    // We then try to load the library at run time and locate the symbols used.
    try {
        // Here we import at runtime the worhp library and protect the whole try block with a mutex
        std::lock_guard<std::mutex> lock(detail::library_load_mutex);
        if (!boost::filesystem::is_regular_file(library_filename)) {
            pagmo_throw(std::invalid_argument,
                        "The worhp library file name was constructed to be: " + library_filename.string()
                            + " and it does not appear to be a file");
        }
        boost::dll::shared_library libworhp(library_filename);
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
        ReadParams = boost::dll::import<void(int *, const char[], Params *)>( // type of the function to import
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
        DoneUserAction = boost::dll::import<void(Control *, int)>( // type of the function to import
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
        StatusMsgString = boost::dll::import<void(OptVar *, Workspace *, Params *, Control *,
                                                  char message[])>( // type of the function to import
            libworhp,                                               // the library
            "StatusMsgString"                                       // name of the function to import
        );
        WorhpSetBoolParam = boost::dll::import<bool(Params *, const char *, bool)>( // type of the function to import
            libworhp,                                                               // the library
            "WorhpSetBoolParam"                                                     // name of the function to import
        );
        WorhpSetIntParam = boost::dll::import<bool(Params *, const char *, int)>( // type of the function to import
            libworhp,                                                             // the library
            "WorhpSetIntParam"                                                    // name of the function to import
        );
        WorhpSetDoubleParam
            = boost::dll::import<bool(Params *, const char *, double)>( // type of the function to import
                libworhp,                                               // the library
                "WorhpSetDoubleParam"                                   // name of the function to import
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
        WorhpVersion = boost::dll::import<void(int *major, int *minor,
                                               char patch[PATCH_STRING_LENGTH])>( // type of the function to import
            libworhp,                                                             // the library
            "WorhpVersion"                                                        // name of the function to import
        );
    } catch (const std::exception &e) {
        std::string message(
            R"(
An error occurred while loading the worhp library at run-time. This is typically caused by one of the following
reasons:

- The file declared to be the worhp library, i.e. )"
            + m_worhp_library
            + R"(, is not found or is found but it is not a shared library containing the necessary symbols 
(is the file really a valid shared library?)
 - The library is found and it does contain the symbols, but it needs linking to some additional libraries that are not found
at run-time.

We report the exact text of the original exception thrown:

 )" + std::string(e.what()));
        pagmo_throw(std::invalid_argument, message);
    }
    // ------------------------- END WORHP PLUGIN -------------------------------------------------------------

    // We check for a version mismatch
    // First we query the library
    int major, minor;
    char patch[PATCH_STRING_LENGTH];
    WorhpVersion(&major, &minor, patch);
    std::string patchstr(patch);
    // Then we check with the pnf headers
    if (major != WORHP_MAJOR || minor != WORHP_MINOR) {
        pagmo_throw(std::invalid_argument, "Your WORHP library (" + library_filename.string()
                                               + ") version is: " + std::to_string(major) + "." + std::to_string(minor)
                                               + " while pagmo plugins nonfree supports only version: "
                                               + std::to_string(WORHP_MAJOR) + "." + std::to_string(WORHP_MINOR));
    }

    // All is good, proceed
    m_log.clear();
    auto fevals0 = prob.get_fevals();

    // With reference to the worhp User Manual (V1.12)
    // USI-0:  Call WorhpPreInit to properly initialise the (empty) data structures.
    OptVar opt;
    Workspace wsp;
    Params par;
    Control cnt;
    WorhpPreInit(&opt, &wsp, &par, &cnt);

    // USI-1: Read parameters from XML
    // Note that a file named "param.xml" will be searched in the current directory only if the environment variable
    // WORHP_PARAM_FILE is not set. Otherwise the WORHP_PARAM_FILE will be used. The number of parameters that are
    // not getting default values will be stored in n_xml_param
    int n_xml_param;
    if (m_verbosity) { // pagmo log is active
        ReadParams(&n_xml_param, const_cast<char *>("param.xml"), &par);
        SetWorhpPrint(detail::no_screen_output);
    } else {
        if (!m_screen_output) { // pagmo log is active
            SetWorhpPrint(detail::no_screen_output);
        }
        ReadParams(&n_xml_param, const_cast<char *>("param.xml"), &par);
    }

    // USI-2: Specify problem dimensions
    opt.n = static_cast<int>(dim);
    opt.m = static_cast<int>(prob.get_nc()); // number of constraints
    auto n_eq = prob.get_nec();
    // Get the sparsity pattern of the gradient
    auto pagmo_gs = prob.gradient_sparsity();
    // Determine where the gradients of the constraints start in the fitness gradient.
    const auto it = std::lower_bound(pagmo_gs.begin(), pagmo_gs.end(), sparsity_pattern::value_type(1u, 0u));
    // Split the sparsity into f and g parts
    sparsity_pattern fs(pagmo_gs.begin(), it);
    sparsity_pattern gs(it, pagmo_gs.end());
    // Create the corresponding index map between pagmo and worhp sparse representation of the gradient
    std::vector<vector_double::size_type> gs_idx_map(gs.size());
    std::iota(gs_idx_map.begin(), gs_idx_map.end(), 0);
    std::sort(gs_idx_map.begin(), gs_idx_map.end(),
              [&gs](const std::vector<vector_double::size_type>::size_type &idx1,
                    const std::vector<vector_double::size_type>::size_type &idx2) -> bool {
                  return (gs[idx1].second < gs[idx2].second
                          || (!(gs[idx2].second < gs[idx1].second) && gs[idx1].first < gs[idx2].first));
              });

    // NOTE: Worhp requires a single sparsity pattern for the hessian of the lagrangian (that is,
    // the pattern must be valid for objfun and all constraints), but we provide a separate sparsity pattern for
    // objfun and every constraint. We will thus need to merge our sparsity patterns in a single sparsity
    // pattern.
    sparsity_pattern merged_hs;
    // Store the original hessians sparsity only if it is user-provided.
    auto hs = prob.hessians_sparsity();
    if (prob.has_hessians_sparsity()) {
        for (const auto &sp : hs) {
            // NOTE: we need to create a separate copy each time as std::set_union() requires distinct ranges.
            const auto old_merged_hs(merged_hs);
            merged_hs.clear();
            std::set_union(old_merged_hs.begin(), old_merged_hs.end(), sp.begin(), sp.end(),
                           std::back_inserter(merged_hs));
        }
    } else {
        // If the hessians sparsity is not user-provided, dense patterns are assumed.
        merged_hs = pagmo::detail::dense_hessian(prob.get_nx());
    }
    // -------------------------------------------------------------------------------------------------------------------------
    /*
     * In WORHP the HM sparsity requires lower triangular entries first,
     * then all the diagonal elements (also the zeros) (cani maledetti^2)
     */
    // Create the corresponding index map between pagmo and worhp sparse representation of the lower triangular part
    // of the hessian of the lagrangian
    std::vector<vector_double::size_type> hs_idx_map(merged_hs.size());
    std::iota(hs_idx_map.begin(), hs_idx_map.end(), 0);
    // Sort the resulting hessian of the lagrangian sparsity according to worhp twisted choice.
    // Lexicographic from right to left, i.e. ((1,0),(2,0),(0,1), )
    std::sort(hs_idx_map.begin(), hs_idx_map.end(),
              [&merged_hs](const vector_double::size_type &idx1, const vector_double::size_type &idx2) -> bool {
                  return (merged_hs[idx1].second < merged_hs[idx2].second
                          || (!(merged_hs[idx2].second < merged_hs[idx1].second)
                              && merged_hs[idx1].first < merged_hs[idx2].first));
              });

    // We remove the diagonal entries from the hessian sparsity as merged from pagmo (if present)
    auto it2 = std::remove_if(hs_idx_map.begin(), hs_idx_map.end(),
                              [&merged_hs](std::vector<vector_double::size_type>::size_type &idx) -> bool {
                                  return (merged_hs[idx].first == merged_hs[idx].second);
                              });
    hs_idx_map.erase(it2, hs_idx_map.end());

    wsp.DF.nnz = static_cast<int>(fs.size());
    wsp.DG.nnz = static_cast<int>(gs.size());
    wsp.HM.nnz = static_cast<int>(hs_idx_map.size() + dim); // lower triangular sparse + full diagonal

    // USI-3 (and 8): Allocate solver memory (and deallocate upon destruction of wr)
    detail::worhp_raii wr(&opt, &wsp, &par, &cnt, WorhpInit, WorhpFree);

    // This flag informs Worhp that f and g should not be evaluated seperately. pagmo fitness always computes both
    // so that if only the objfun is needed also the constraints are computed. This flag signals to worhp that this
    // is the case. Since the flag makes sense only for constrained problems, we set it only if necessary (worhp
    // would otherwise print a warning)
    if (prob.get_nc() > 0) {
        par.FGtogether = true;
    }

    // We deal with the gradient
    if (prob.has_gradient()) {
        WorhpSetBoolParam(&par, "UserDF", true);
        WorhpSetBoolParam(&par, "UserDG", true);
    } else {
        WorhpSetBoolParam(&par, "UserDF", false);
        WorhpSetBoolParam(&par, "UserDG", false);
    }
    if (prob.has_hessians()) {
        WorhpSetBoolParam(&par, "UserHM", true);
    } else {
        WorhpSetBoolParam(&par, "UserHM", false);
    }

    // Logic for the handling of constraints tolerances. The logic is as follows:
    // - if the user provides the "TolFeas" option, use that *unconditionally*. Otherwise,
    // - compute the minimum tolerance min_tol among those returned by  problem.c_tol(). If zero, ignore
    //   it and use the WORHP default value for "TolFeas" (1e-6). Otherwise, use min_tol as
    //   the value for "TolFeas" and min_tol/2 for AcceptTolFeas
    if (prob.get_nc() && !m_numeric_opts.count("TolFeas")) {
        const auto c_tol = prob.get_c_tol();
        assert(!c_tol.empty());
        const double min_tol = *std::min_element(c_tol.begin(), c_tol.end());
        if (min_tol > 0.) {
            auto res = WorhpSetDoubleParam(&par, "TolFeas", min_tol);
            res = WorhpSetDoubleParam(&par, "AcceptTolFeas", min_tol / 2);
            assert(res == true);
        }
    }

    // We now set the user defined options
    // floats
    for (const auto &p : m_numeric_opts) {
        auto success = WorhpSetDoubleParam(&par, p.first.c_str(), p.second);
        if (!success) {
            pagmo_throw(std::invalid_argument,
                        "The option '" + p.first + "' was requested by the user to be set to the float value "
                            + std::to_string(p.second)
                            + ", but WORHP interface returned an error. Did you mispell the option name?");
        }
    }
    // int
    for (const auto &p : m_integer_opts) {
        auto success = WorhpSetIntParam(&par, p.first.c_str(), p.second);
        if (!success) {
            pagmo_throw(std::invalid_argument,
                        "The option '" + p.first + "' was requested by the user to be set to the integer value "
                            + std::to_string(p.second)
                            + ", but WORHP interface returned an error. Did you mispell the option name?");
        }
    }
    // bool
    for (const auto &p : m_bool_opts) {
        auto success = WorhpSetBoolParam(&par, p.first.c_str(), p.second);
        if (!success) {
            pagmo_throw(std::invalid_argument,
                        "The option '" + p.first + "' was requested by the user to be set to the bool value "
                            + std::to_string(p.second)
                            + ", but WORHP interface returned an error. Did you mispell the option name?");
        }
    }

    // USI-5: Set initial values and deal with gradients / hessians
    // We define the initial value for the chromosome
    // We init the starting point using the inherited methods from not_population_based
    auto sel_xf = select_individual(pop);
    vector_double x0(std::move(sel_xf.first)), f0(std::move(sel_xf.second)); // TODO: is f0 useful to worhp?
    for (vector_double::size_type i = 0u; i < static_cast<vector_double::size_type>(opt.n); ++i) {
        opt.X[i] = x0[i];
    }
    opt.F = wsp.ScaleObj * f0[0];
    for (vector_double::size_type i = 0u; i < static_cast<vector_double::size_type>(opt.m); ++i) {
        opt.G[i] = f0[i + 1];
    }

    // USI-6: Set the constraint bounds
    // Box bounds
    for (vector_double::size_type i = 0; i < static_cast<vector_double::size_type>(opt.n); ++i) {
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
    for (auto i = n_eq; i < static_cast<decltype(n_eq)>(opt.m); ++i) {
        opt.Mu[i] = 0;
        opt.GL[i] = -par.Infty;
        opt.GU[i] = 0;
    }

    /*
     * Specify matrix structures in CS format, using Fortran indexing,
     * i.e. 1...N instead of 0...N-1, to describe the matrix structure.
     * Only if the declared size is not dense.
     */
    // -------------------------------------------------------------------------------------------------------------------------
    // Assign sparsity structure to DF
    if (wsp.DF.NeedStructure) {
        for (decltype(fs.size()) i = 0; i < fs.size(); ++i) {
            // NOTE: the +1 is because of fortran notation is required by WORHP (maledetti).
            wsp.DF.row[i] = static_cast<int>(fs[i].second + 1);
        }
    }
    // -------------------------------------------------------------------------------------------------------------------------
    // Assign sparsity structure to DG if not dense.
    if (wsp.DG.NeedStructure) {
        for (decltype(gs_idx_map.size()) i = 0u; i < gs_idx_map.size(); ++i) {
            // NOTE: no need for +1 here as in pagmo 0 is the objfun already stripped from here.
            wsp.DG.row[i] = static_cast<int>(gs[gs_idx_map[i]].first);
            // NOTE: the +1 is because of fortran notation is required by WORHP (maledetti).
            wsp.DG.col[i] = static_cast<int>(gs[gs_idx_map[i]].second + 1);
        }
    }
    // -------------------------------------------------------------------------------------------------------------------------
    // Assign sparsity structure to HM if not dense. (this requires to perform the same operations as above,
    // but directly on the merged_hs not on the iota)
    if (wsp.HM.NeedStructure) {
        // Strict lower triangle
        for (decltype(hs_idx_map.size()) i = 0u; i < hs_idx_map.size(); ++i) {
            // NOTE: the +1 is because fortran notation is required by WORHP (maledetti).
            wsp.HM.row[i] = static_cast<int>(merged_hs[hs_idx_map[i]].first + 1);
            // NOTE: the +1 is because fortran notation is required by WORHP (maledetti).
            wsp.HM.col[i] = static_cast<int>(merged_hs[hs_idx_map[i]].second + 1);
        }

        // Diagonal
        for (decltype(dim) i = 0; i < dim; ++i) {
            wsp.HM.row[hs_idx_map.size() + i] = static_cast<int>(i + 1);
            wsp.HM.col[hs_idx_map.size() + i] = static_cast<int>(i + 1);
        }
    }
    // -------------------------------------------------------------------------------------------------------------------------

    if (m_verbosity) {
        print("WORHP version is (library): ", major, ".", minor, ".", patchstr, "\n");
        print("WORHP version is (plugin headers): ", WORHP_VERSION, "\n");
        print("\nWORHP plugin for pagmo/pygmo: \n");
        if (prob.has_gradient_sparsity()) {
            print("\tThe gradient sparsity is provided by the user: ", pagmo_gs.size(), " components detected.\n");
        } else {
            print("\tThe gradient sparsity is assumed dense: ", pagmo_gs.size(), " components detected.\n");
        }
        if (prob.has_gradient()) {
            print("\tThe gradient is provided by the user.\n");
        } else {
            print("\tThe gradient is computed numerically by WORHP.\n");
        }
        print("\tThe hessian of the lagrangian sparsity has: ", merged_hs.size(), " components.\n");

        if (prob.has_hessians()) {
            print("\tThe hessians are provided by the user.\n");
        } else {
            print("\tThe hessian of the lagrangian is computed numerically by WORHP.\n");
        }
        print("\nThe following parameters have been set by pagmo to values other than their xml provided ones (or "
              "their default ones): \n");
        print("\tpar.FGtogether: ", par.FGtogether, "\n");
        print("\tpar.UserDF: ", par.UserDF, "\n");
        print("\tpar.UserDG: ", par.UserDG, "\n");
        print("\tpar.UserHM: ", par.UserHM, "\n");
        print("\tpar.TolFeas: ", par.UserHM, "\n");
        print("\tpar.AcceptTolFeas: ", par.UserHM, "\n");
        // floats
        for (const auto &p : m_numeric_opts) {
            print("\tpar.", p.first, ": ", p.second, "\n");
        }
        // int
        for (const auto &p : m_integer_opts) {
            print("\tpar.", p.first, ": ", p.second, "\n");
        }
        // bool
        for (const auto &p : m_bool_opts) {
            print("\tpar.", p.first, ": ", p.second, "\n");
        }

        print("\n", std::setw(10), "objevals:", std::setw(15), "objval:", std::setw(15), "violated:", std::setw(15),
              "viol. norm:", '\n');
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
            UserF(&opt, &wsp, &par, &cnt, pop, fevals0);
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
         * Evaluate the Hessian matrix of the Lagrange function (L = f + mu*g)
         * The call to UserHM may be replaced by user-defined code.
         */
        if (GetUserAction(&cnt, evalHM)) {
            UserHM(&opt, &wsp, &par, &cnt, pop, hs, merged_hs, hs_idx_map);
            DoneUserAction(&cnt, evalHM);
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
    for (vector_double::size_type i = 0u; i < static_cast<vector_double::size_type>(opt.n); ++i) {
        x_final[i] = opt.X[i];
    }

    f_final = prob.fitness(x_final);

    if (compare_fc(f_final, f0, prob.get_nec(), prob.get_c_tol())) {
        replace_individual(pop, x_final, f_final);
    }

    // We retrieve the text of the optimization result
    char cstr[1024];
    StatusMsgString(&opt, &wsp, &par, &cnt, cstr);

    m_last_opt_res = std::string(cstr);

    // And print it to screen if requested
    if (m_verbosity) {
        print(m_last_opt_res, "\n");
    } else if (m_screen_output) {
        StatusMsg(&opt, &wsp, &par, &cnt);
    }

    return pop;
}

/// Set verbosity.
/**
 * This method will set the algorithm's verbosity. If \p n is zero, no output is produced during the
 * optimisation by pagmo and no logging is performed. If \p n is nonzero, then every \p n objective function
 * evaluations the status of the optimisation will be both printed to screen and recorded internally. See
 * worhp::log_line_type and worhp::log_type for information on the logging format. The internal log can be fetched
 * via get_log().
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
 *    That is, they might not necessarily be consistent with WORHP's notion of feasibility.
 *
 * .. note::
 *
 *    WORHP supports its own logging format and protocol, including the ability to print to screen and write to
 *    file. WORHP's screen logging is disabled by default. On-screen logging can be enabled constructing the
 *    object ppnf::WORHP passing ``true`` as argument. In this case verbosity will not be allowed to be set.
 *
 * \endverbatim
 *
 */
void worhp::set_verbosity(unsigned n)
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
 * See worhp::log_type for a description of the optimisation log. Logging is turned on/off via
 * set_verbosity().
 *
 * @return a const reference to the log.
 */
const worhp::log_type &worhp::get_log() const
{
    return m_log;
}
/// Gets the verbosity level
/**
 * @return the verbosity level
 */
unsigned int worhp::get_verbosity() const
{
    return m_verbosity;
}
/// Algorithm name
/**
 * One of the optional methods of any user-defined algorithm (UDA).
 *
 * @return a string containing the algorithm name
 */
std::string worhp::get_name() const
{
    return "WORHP";
}
/// Get extra information about the algorithm.
/**
 * @return a human-readable string containing useful information about the algorithm's properties
 * (e.g., the WORHP user-set options, the selection/replacement policies, etc.), the worhp library path
 */
std::string worhp::get_extra_info() const
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
    if (m_integer_opts.size()) {
        stream(ss, "\n\tInteger options: ", pagmo::detail::to_string(m_integer_opts));
    }
    if (m_numeric_opts.size()) {
        stream(ss, "\n\tNumeric options: ", pagmo::detail::to_string(m_numeric_opts));
    }
    if (m_bool_opts.size()) {
        stream(ss, "\n\\tBoolean options: ", pagmo::detail::to_string(m_bool_opts));
    }
    stream(ss, "\n");
    stream(ss, "\nLast optimisation result: \n", m_last_opt_res);
    stream(ss, "\n");
    return ss.str();
}

/// Set integer option.
/**
 * This method will set the optimisation integer option \p name to \p value.
 * The optimisation options are passed to the WORHP API when calling evolve().
 *
 * @param name of the option.
 * @param value of the option.
 */
void worhp::set_integer_option(const std::string &name, int value)
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
void worhp::set_integer_options(const std::map<std::string, int> &m)
{
    for (const auto &p : m) {
        set_integer_option(p.first, p.second);
    }
}
/// Get integer options.
/**
 * @return the name-value map of optimisation integer options.
 */
std::map<std::string, int> worhp::get_integer_options() const
{
    return m_integer_opts;
}
/// Set numeric option.
/**
 * This method will set the optimisation numeric option \p name to \p value.
 * The optimisation options are passed to the WORHP API when calling evolve().
 *
 * @param name of the option.
 * @param value of the option.
 */
void worhp::set_numeric_option(const std::string &name, double value)
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
void worhp::set_numeric_options(const std::map<std::string, double> &m)
{
    for (const auto &p : m) {
        set_numeric_option(p.first, p.second);
    }
}
/// Get numeric options.
/**
 * @return the name-value map of optimisation numeric options.
 */
std::map<std::string, double> worhp::get_numeric_options() const
{
    return m_numeric_opts;
}
/// Set bool option.
/**
 * This method will set the optimisation integer option \p name to \p value.
 * The optimisation options are passed to the WORHP API when calling evolve().
 *
 * @param name of the option.
 * @param value of the option.
 */
void worhp::set_bool_option(const std::string &name, bool value)
{
    m_bool_opts[name] = value;
}
/// Set bool options.
/**
 * This method will set the optimisation integer options contained in \p m.
 * It is equivalent to calling set_bool_option() passing all the name-value pairs in \p m
 * as arguments.
 *
 * @param m the name-value map that will be used to set the options.
 */
void worhp::set_bool_options(const std::map<std::string, bool> &m)
{
    for (const auto &p : m) {
        set_bool_option(p.first, p.second);
    }
}
/// Get bool options.
/**
 * @return the name-value map of optimisation integer options.
 */
std::map<std::string, bool> worhp::get_bool_options() const
{
    return m_bool_opts;
}
/// Clear all integer options.
void worhp::reset_integer_options()
{
    m_integer_opts.clear();
}
/// Clear all numeric options.
void worhp::reset_numeric_options()
{
    m_numeric_opts.clear();
}
/// Clear all numeric options.
void worhp::reset_bool_options()
{
    m_bool_opts.clear();
}
/// Get the result of the last optimisation.
/**
 * @return the result of the last call to WORHP. You can check
 * The WORHP user manual for the meaning of the various entries.
 * \verbatim embed:rst:leading-asterisk
 *
 * .. seealso::
 *
 *    https://worhp.de/latest/download/user_manual.pdf
 *
 * \endverbatim
 */
std::string worhp::get_last_opt_result() const
{
    return m_last_opt_res;
}

// Log update and print to screen
void worhp::update_log(const problem &prob, const vector_double &fit, long long unsigned fevals0) const
{
    unsigned fevals = static_cast<unsigned>(prob.get_fevals() - fevals0);
    if (m_verbosity && !(fevals % m_verbosity)) {
        // Constraints bits.
        const auto ctol = prob.get_c_tol();
        const auto c1eq
            = pagmo::detail::test_eq_constraints(fit.data() + 1, fit.data() + 1 + prob.get_nec(), ctol.data());
        const auto c1ineq = pagmo::detail::test_ineq_constraints(fit.data() + 1 + prob.get_nec(),
                                                                 fit.data() + fit.size(), ctol.data() + prob.get_nec());
        // This will be the total number of violated constraints.
        const auto nv = prob.get_nc() - c1eq.first - c1ineq.first;
        // This will be the norm of the violation.
        const auto l = c1eq.second + c1ineq.second;
        // Test feasibility.
        const auto feas = prob.feasibility_f(fit);

        if (!(fevals / m_verbosity % 50u)) {
            // Every 50 lines print the column names.
            print("\n", std::setw(10), "objevals:", std::setw(15), "objval:", std::setw(15), "violated:", std::setw(15),
                  "viol. norm:", '\n');
        }
        // Print to screen the log line.
        print(std::setw(10), fevals, std::setw(15), fit[0], std::setw(15), nv, std::setw(15), l, feas ? "" : " i",
              '\n');
        // Record the log.
        m_log.emplace_back(fevals, fit[0], nv, l, feas);
    }
}

// Objective function
void worhp::UserF(OptVar *opt, Workspace *wsp, Params *, Control *, const population &pop,
                  long long unsigned fevals0) const
{
    double *X = opt->X; // Abbreviate notation
    const auto &prob = pop.get_problem();
    auto dim = prob.get_nx();
    vector_double x(X, X + dim);
    auto fit = fitness_with_cache(x, prob);
    update_log(prob, fit, fevals0);
    opt->F = wsp->ScaleObj * fit[0];
}
// Constraints
void worhp::UserG(OptVar *opt, Workspace *, Params *, Control *, const population &pop) const
{
    double *X = opt->X; // Abbreviate notation
    const auto &prob = pop.get_problem();
    auto dim = prob.get_nx();
    vector_double x(X, X + dim);
    auto fit = fitness_with_cache(x, prob);
    for (decltype(prob.get_nc()) i = 0; i < prob.get_nc(); ++i) {
        opt->G[i] = fit[i + 1];
    }
}
// Gradient for the objective function
void worhp::UserDF(OptVar *opt, Workspace *wsp, Params *, Control *, const population &pop) const
{
    const auto &prob = pop.get_problem();
    auto dim = prob.get_nx();
    vector_double x(opt->X, opt->X + dim);
    auto g = gradient_with_cache(x, prob);
    for (vector_double::size_type i = 0u; i < static_cast<vector_double::size_type>(wsp->DF.nnz); ++i) {
        wsp->DF.val[i] = g[i];
    }
}

// Gradient for the constraints
void worhp::UserDG(OptVar *opt, Workspace *wsp, Params *, Control *, const population &pop,
                   const std::vector<vector_double::size_type> &gs_idx_map) const
{
    const auto &prob = pop.get_problem();
    auto dim = prob.get_nx();
    vector_double x(opt->X, opt->X + dim);
    auto g = gradient_with_cache(x, prob);
    for (vector_double::size_type i = 0u; i < static_cast<vector_double::size_type>(wsp->DG.nnz); ++i) {
        wsp->DG.val[i] = g[static_cast<vector_double::size_type>(wsp->DF.nnz) + gs_idx_map[i]];
    }
}

// The Hessian of the Lagrangian L = f + mu * g
void worhp::UserHM(OptVar *opt, Workspace *wsp, Params *, Control *, const population &pop,
                   const std::vector<sparsity_pattern> &pagmo_hsp, const sparsity_pattern &pagmo_merged_hsp,
                   const std::vector<vector_double::size_type> &hs_idx_map) const
{
    const auto &prob = pop.get_problem();
    auto dim = prob.get_nx();
    vector_double x(opt->X, opt->X + dim);
    auto pagmo_h = prob.hessians(x);
    // Compute the hessian of the lagrangian. Logic: first we assemble the Hessian of the Lagrangian
    // as represented by an unordered map (i,j) - > valij. We do so looping on the pagmo hessians
    // and inserting the various contributions where they belong. Later we transform this representation
    // into the worhp representation.
    std::unordered_map<std::pair<vector_double::size_type, vector_double::size_type>, double, pair_hash> pagmo_merged_h;
    // First we deal with the objective
    for (decltype(pagmo_hsp[0].size()) j = 0u; j < pagmo_hsp[0].size(); ++j) {
        // These will all be insertions in the map as all keys will not be there.
        pagmo_merged_h[pagmo_hsp[0][j]] = pagmo_h[0][j] * wsp->ScaleObj;
    }
    // Then with the constraints
    for (decltype(pagmo_hsp.size()) i = 1u; i < pagmo_hsp.size(); ++i) {
        for (decltype(pagmo_hsp[i].size()) j = 0u; j < pagmo_hsp[i].size(); ++j) {
            // If the key is there, great! Otherwise a 0 will be created and pagmo_h[i][j] * opt->Mu[i-1] summed
            // over.
            pagmo_merged_h[pagmo_hsp[i][j]] = pagmo_merged_h[pagmo_hsp[i][j]] + pagmo_h[i][j] * opt->Mu[i - 1];
        }
    }
    // At this point the hessian of the lagrangian is assembled in pagmo_merged_h
    // but we still need to translate it into the WORHP format.
    // lower triangular
    for (decltype(hs_idx_map.size()) i = 0u; i < hs_idx_map.size(); ++i) {
        wsp->HM.val[i] = pagmo_merged_h[pagmo_merged_hsp[hs_idx_map[i]]];
    }
    // diagonal
    for (decltype(dim) i = 0u; i < dim; ++i) {
        wsp->HM.val[hs_idx_map.size() + i] = pagmo_merged_h[{i, i}];
    }
}

// We cache the last call to fitness as it will be repeated by worhp
vector_double worhp::fitness_with_cache(const vector_double &x, const problem &prob) const
{
    if (x == m_f_cache.first) {
        return m_f_cache.second;
    } else {
        vector_double fit = prob.fitness(x);
        m_f_cache = std::pair<vector_double, vector_double>{x, fit};
        return fit;
    }
}

// We cache the last call to gradient as it will be repeated by worhp
vector_double worhp::gradient_with_cache(const vector_double &x, const problem &prob) const
{
    if (x == m_g_cache.first) {
        return m_g_cache.second;
    } else {
        vector_double grad = prob.gradient(x);
        m_g_cache = std::pair<vector_double, vector_double>{x, grad};
        return grad;
    }
}

} // namespace ppnf

PAGMO_S11N_ALGORITHM_IMPLEMENT(ppnf::worhp)
