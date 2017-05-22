#include <pygmo/python_includes.hpp>

// #define PY_ARRAY_UNIQUE_SYMBOL snopt7_ARRAY_API
#include <pygmo/numpy.hpp>

#include <boost/python/import.hpp>
#include <boost/python/module.hpp>
#include <pagmo-snopt7/snopt7.hpp>
#include <pagmo/population.hpp>

#include <pygmo/algorithm_exposition_suite.hpp>
#include <pygmo/pygmo_classes.hpp>

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

    wrap_import_array();

    auto pygmo_module = bp::import("pygmo");

    // auto my_uda_ = pygmo::expose_algorithm<my_uda>("my_uda", "My UDA.");

    auto &algorithm_class = *pygmo::algorithm_ptr;
    // We require all algorithms to be def-ctible at the bare minimum.
    bp::class_<pagmo::snopt7> c("snopt7", "", bp::init<>());
    // Mark it as a C++ algorithm.
    c.attr("_pygmo_cpp_algorithm") = true;

    // Expose the algorithm constructor from Algo.
    pygmo::algorithm_expose_init_cpp_uda<pagmo::snopt7>();
    // Expose extract.
    algorithm_class.def("_cpp_extract", &pygmo::generic_cpp_extract<pagmo::algorithm, pagmo::snopt7>,
                        bp::return_internal_reference<>());

    // Add the algorithm to the algorithms submodule.
    // bp::scope().attr("algorithms").attr(name) = c;
}