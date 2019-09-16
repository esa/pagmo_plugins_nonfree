#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>

#include <pygmo/expose_algorithms.hpp>
#include <pygmo/algorithm_exposition_suite.hpp>
#include <pygmo/register_ap.hpp>

#include <pagmo_plugins_nonfree/snopt7.hpp>
#include <pagmo_plugins_nonfree/worhp.hpp>

#include "docstrings.hpp"

namespace bp = boost::python;
namespace pg = pygmo;


BOOST_PYTHON_MODULE(core)
{
    // Setup doc options
    bp::docstring_options doc_options;
    doc_options.enable_all();
    doc_options.disable_cpp_signatures();
    doc_options.disable_py_signatures();

    // Registers the affiliated package so that upon import will add itself to the cereal serialization dictionary
    pg::register_ap();

    auto snopt7_ = pg::expose_algorithm<pagmo::snopt7>("snopt7", pg::snopt7_docstring().c_str());
    // We expose the additional constructor
    snopt7_.def(
        bp::init<bool, std::string, unsigned>((bp::arg("screen_output") = false, bp::arg("library") = "/usr/local/lib/", bp::arg("minor_version") = 6)));
    snopt7_.def("set_integer_option", &pagmo::snopt7::set_integer_option,
                pg::snopt7_set_integer_option_docstring().c_str(), (bp::arg("name"), bp::arg("value")));
    snopt7_.def("set_numeric_option", &pagmo::snopt7::set_numeric_option,
                pg::snopt7_set_numeric_option_docstring().c_str(), (bp::arg("name"), bp::arg("value")));
    pg::expose_algo_log(snopt7_, pg::snopt7_get_log_docstring().c_str());
    //uncomment when expose_not_population_based will be on the public interface of pygmo
    //pg::expose_not_population_based(snopt7_, "snopt7");

    auto worhp_ = pg::expose_algorithm<pagmo::worhp>("worhp", pg::worhp_docstring().c_str());
    // We expose the additional constructor
    worhp_.def(bp::init<bool, std::string>((bp::arg("screen_output") = false, bp::arg("library") = "/usr/local/lib/")));
    worhp_.def("set_integer_option", &pagmo::worhp::set_integer_option,
               pg::worhp_set_integer_option_docstring().c_str(), (bp::arg("name"), bp::arg("value")));
    worhp_.def("set_numeric_option", &pagmo::worhp::set_numeric_option,
               pg::worhp_set_numeric_option_docstring().c_str(), (bp::arg("name"), bp::arg("value")));
    worhp_.def("set_bool_option", &pagmo::worhp::set_bool_option, pg::worhp_set_bool_option_docstring().c_str(),
               (bp::arg("name"), bp::arg("value")));
    pg::expose_algo_log(worhp_, pg::worhp_get_log_docstring().c_str());
    //pg::expose_not_population_based(worhp_, "worhp");
}
