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

// This is necessary because the NumPy macro import_array() has different return
// values
// depending on the Python version.
#if PY_MAJOR_VERSION < 3
static inline void wrap_import_array()
{
    import_array();
}
#else
static inline void *wrap_import_array()
{
    import_array();
    return nullptr;
}
#endif

namespace bp = boost::python;

BOOST_PYTHON_MODULE(core)
{
    // This function needs to be called before doing anything with threads.
    // https://docs.python.org/3/c-api/init.html
    ::PyEval_InitThreads();

    // Setup doc options
    bp::docstring_options doc_options;
    doc_options.enable_all();
    doc_options.disable_cpp_signatures();
    doc_options.disable_py_signatures();

    wrap_import_array();

    auto pygmo_module = bp::import("pygmo");

    auto &algorithm_class = pygmo::get_algorithm_class();
    // We require all algorithms to be def-ctible at the bare minimum.
    bp::class_<pagmo::snopt7> snopt7_("snopt7", pygmo::snopt7_docstring().c_str(), bp::init<>());
    snopt7_.def(bp::init<bool, std::string>(
        (bp::arg("screen_output") = false, bp::arg("absolute_lib_path") = "/usr/local/lib/")));
    // Mark it as a C++ algorithm.
    snopt7_.attr("_pygmo_cpp_algorithm") = true;
    pygmo::expose_algo_log(snopt7_, pygmo::snopt7_get_log_docstring().c_str());

    // Expose the algorithm constructor from Algo.
    algorithm_class.def("_cpp_extract", &pygmo::generic_cpp_extract<pagmo::algorithm, pagmo::snopt7>,
                        bp::return_internal_reference<>());
}