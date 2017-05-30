#include <pygmo/python_includes.hpp>

#include <pygmo/numpy.hpp>

#include <boost/python/docstring_options.hpp>
#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <pagmo/population.hpp>
#include <pagmo_plugins_nonfree/snopt7.hpp>
#include <pygmo/algorithm_exposition_suite.hpp>
#include <pygmo/pygmo_classes.hpp>

#include "docstrings.hpp"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(core)
{
    // Setup doc options
    bp::docstring_options doc_options;
    doc_options.enable_all();
    doc_options.disable_cpp_signatures();
    doc_options.disable_py_signatures();

    auto snopt7_ = pygmo::expose_algorithm<pagmo::snopt7>("snopt7", pygmo::snopt7_docstring().c_str());
    // We expose the additional constructor
    snopt7_.def(bp::init<bool, std::string>(
        (bp::arg("screen_output") = false, bp::arg("absolute_lib_path") = "/usr/local/lib/")));
}