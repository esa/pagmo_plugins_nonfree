#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>

#include <pygmo/algorithm_exposition_suite.hpp>
#include <pygmo/register_ap.hpp>

#include <pagmo_plugins_nonfree/snopt7.hpp>

#include "docstrings.hpp"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(core)
{
    // Setup doc options
    bp::docstring_options doc_options;
    doc_options.enable_all();
    doc_options.disable_cpp_signatures();
    doc_options.disable_py_signatures();

    // Registers the affiliated package so that upon import will add itself to the cereal serialization dictionary
    pygmo::register_ap();

    auto snopt7_ = pygmo::expose_algorithm<pagmo::snopt7>("snopt7", pygmo::snopt7_docstring().c_str());
    // We expose the additional constructor
    snopt7_.def(bp::init<bool, std::string>(
        (bp::arg("screen_output") = false, bp::arg("absolute_lib_path") = "/usr/local/lib/")));
    snopt7_.def("set_integer_option", &pagmo::snopt7::set_integer_option, pygmo::snopt7_set_integer_option_docstring().c_str(), (bp::arg("name"), bp::arg("value")));
    snopt7_.def("set_numeric_option", &pagmo::snopt7::set_numeric_option, pygmo::snopt7_set_numeric_option_docstring().c_str(), (bp::arg("name"), bp::arg("value")));
}
