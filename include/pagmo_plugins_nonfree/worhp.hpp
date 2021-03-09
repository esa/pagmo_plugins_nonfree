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

#ifndef PAGMO_WORHP_HPP
#define PAGMO_WORHP_HPP

#include <algorithm> // std::min_element, std::sort, std::remove_if
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/map.hpp>
#include <iomanip>
#include <memory>
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

#include "bogus_libs/worhp_lib/worhp_bogus.h"
#include <pagmo_plugins_nonfree/detail/visibility.hpp>

namespace ppnf
{

    //forward declaration
    namespace detail {
        struct worhp_raii;
    }

/// WORHP - (We Optimize Really Huge Problems)
/**
 * \image html worhp.png
 *
 * This class is a user-defined algorithm (UDA) that contains a plugin to the WORHP (We Optimize Really Huge Problems)
 * solver (https://worhp.de/), a software package for large-scale nonlinear optimization. WORHP is a powerful solver
 * that is able to handle robustly and efficiently constrained nonlinear opimization problems also at high
 * dimensionalities. The wrapper was developed around the version 1.12 of WORHP and the Full Feature Interface (FFI)
 * using the Unified Solver Interface and the Reverse Communication paradigm (see worhp user manual).
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. note::
 *
 *    The WORHP library is only available acquiring a licence. You can consult the web pages at
 *    (https://worhp.de/) for further information. In thse web pages you will be able to download the correct library
 *    for your architecture and obtain a license file. The WORHP user manual is also available, where the user can
 *    see what options can be set and their intended use. You will be able to specify the location of the downloaded
 *    library when constructing this UDA.
 *
 * \endverbatim
 *
 *
 * Worhp is designed to efficiently solve small- to large-scale constrained optimisation problems (single objective),
 * where the objective function and the constraints are sufficiently smooth, and may be linear, quadratic or nonlinear.
 * It is designed to find locally optimal points of optimisation problems, which may be globally optimal, depending on
 * the problem structure, the initial guess and other factors. Worhp combines  a  Sequential  Quadratic  Programming
 * (SQP) method  on  the general nonlinear level with a primal-dual Interior Point (IP) method on the quadratic
 * subproblem level, to generate a sequence of search directions, which are subject to line search using the Augmented
 * Lagrangian or L1 merit function.
 *
 * Worhp needs first and second order derivatives, which can be supplied by the user, or approximated by finite
 * differences or quasi-Newton methods by WORHP.
 *
 * In order to support pagmo's population-based optimisation model, worhp::evolve() will select
 * a single individual from the input ppnf::population to be optimised.
 * If the optimisation produces an improved individual (as established by ppnf::compare_fc()),
 * the optimised individual will be inserted back into the population.
 * The selection and replacement strategies can be configured via set_selection(const std::string &),
 * set_selection(population::size_type), set_replacement(const std::string &) and
 * set_replacement(population::size_type).
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. note::
 *
 *    This plugin for the WORHP was developed around version 1.12.1 of the worhp library and will not work with
 *    any other version.
 *
 * .. warning::
 *
 *    A moved-from :cpp:class:`ppnf::worhp` is destructible and assignable. Any other operation will result
 *    in undefined behaviour.
 *
 * .. seealso::
 *
 *    https://worhp.de/
 *
 *
 * \endverbatim
 */
class PPNF_DLL_PUBLIC worhp : public pagmo::not_population_based
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
    using log_line_type = std::tuple<unsigned long, double, pagmo::vector_double::size_type, double, bool>;
    /// Log type.
    /**
     * The algorithm log is a collection of worhp::log_line_type data lines, stored in chronological order
     * during the optimisation if the verbosity of the algorithm is set to a nonzero value
     * (see worhp::set_verbosity()).
     */
    using log_type = std::vector<log_line_type>;

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
    worhp(bool screen_output = false, std::string worhp_library = "/usr/local/lib/libworhp.so");

    /**
    * Custom copy constructor, necessary because the worhp data structures hold pointers.
    * It copies the settings, but not the results of the last call to evolve.
    * Evolve needs to be called again before zen_update or zen_get_max_perturbations can be used.
    * 
    * @param other Another instance of worhp
    */ 
    worhp(const worhp& other);

    pagmo::population evolve(pagmo::population pop) const;
    pagmo::vector_double zen_update(const pagmo::vector_double &dr,
                             const pagmo::vector_double &dq, const pagmo::vector_double &db, int order);
    std::vector<pagmo::vector_double> zen_get_max_perturbations();
    void set_verbosity(unsigned n);
    const log_type &get_log() const;
    unsigned int get_verbosity() const;
    std::string get_name() const;
    std::string get_extra_info() const;
    void set_integer_option(const std::string &name, int value);
    void set_integer_options(const std::map<std::string, int> &m);
    std::map<std::string, int> get_integer_options() const;
    void set_numeric_option(const std::string &name, double value);
    void set_numeric_options(const std::map<std::string, double> &m);
    std::map<std::string, double> get_numeric_options() const;
    void set_bool_option(const std::string &name, bool value);
    void set_bool_options(const std::map<std::string, bool> &m);
    std::map<std::string, bool> get_bool_options() const;
    void reset_integer_options();
    void reset_numeric_options();
    void reset_bool_options();
    std::string get_last_opt_result() const;
    /// Object serialization
    /**
     * This method will save/load \p this into the archive \p ar.
     *
     * @param ar target archive.
     *
     * @throws unspecified any exception thrown by the serialization of the UDA and of primitive types.
     */
    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        pagmo::detail::archive(ar, boost::serialization::base_object<not_population_based>(*this), m_worhp_library,
                               m_last_opt_res, m_integer_opts, m_numeric_opts, m_bool_opts, m_screen_output,
                               m_verbosity, m_f_cache, m_g_cache);
    }

private:
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2> &p) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, p.first);
            boost::hash_combine(seed, p.second);
            return seed;
        }
    };
    // Log update and print to screen
    void update_log(const pagmo::problem &prob, const pagmo::vector_double &fit, long long unsigned fevals0) const;
    // Objective function
    void UserF(OptVar *opt, Workspace *wsp, Params *, Control *, const pagmo::population &pop,
               long long unsigned fevals0) const;
    // Constraints
    void UserG(OptVar *opt, Workspace *, Params *, Control *, const pagmo::population &pop) const;
    // Gradient for the objective function
    void UserDF(OptVar *opt, Workspace *wsp, Params *, Control *, const pagmo::population &pop) const;
    // Gradient for the constraints
    void UserDG(OptVar *opt, Workspace *wsp, Params *, Control *, const pagmo::population &pop,
                const std::vector<pagmo::vector_double::size_type> &gs_idx_map) const;
    // The Hessian of the Lagrangian L = f + mu * g
    void UserHM(OptVar *opt, Workspace *wsp, Params *, Control *, const pagmo::population &pop,
                const std::vector<pagmo::sparsity_pattern> &pagmo_hsp, const pagmo::sparsity_pattern &pagmo_merged_hsp,
                const std::vector<pagmo::vector_double::size_type> &hs_idx_map) const;
    // We cache the last call to fitness as it will be repeated by worhp
    pagmo::vector_double fitness_with_cache(const pagmo::vector_double &x, const pagmo::problem &prob) const;
    // We cache the last call to gradient as it will be repeated by worhp
    pagmo::vector_double gradient_with_cache(const pagmo::vector_double &x, const pagmo::problem &prob) const;
    // The absolute path to the worhp library
    std::string m_worhp_library;
    // Solver return status.
    mutable std::string m_last_opt_res
        = "\tThere still is no last optimisation result as WORHP evolve was never successfully called yet.";

    // Options maps.
    std::map<std::string, int> m_integer_opts;
    std::map<std::string, double> m_numeric_opts;
    std::map<std::string, bool> m_bool_opts;

    // Activates the original worhp screen output
    bool m_screen_output;
    unsigned int m_verbosity;
    mutable log_type m_log;

    // The caches
    mutable std::pair<pagmo::vector_double, pagmo::vector_double> m_f_cache = {{}, {}};
    mutable std::pair<pagmo::vector_double, pagmo::vector_double> m_g_cache = {{}, {}};

    // The worhp data structures, caching results from an evolution
    mutable OptVar m_opt;
    mutable Workspace m_wsp;
    mutable Params m_par;
    mutable Control m_cnt;
    mutable std::shared_ptr<detail::worhp_raii> m_wr;

    // Deleting the methods load save public in base as to avoid conflict with serialize
    template <typename Archive>
    void load(Archive &ar) = delete;
    template <typename Archive>
    void save(Archive &ar) const = delete;

};

} // namespace ppnf

PAGMO_S11N_ALGORITHM_EXPORT_KEY(ppnf::worhp)
#endif // PAGMO_WORHP
