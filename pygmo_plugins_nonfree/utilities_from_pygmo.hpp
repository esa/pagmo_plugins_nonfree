#ifndef PYGMO_PLUGINS_NONFREE_UTILITIES_FROM_PYGMO_HPP
#define PYGMO_PLUGINS_NONFREE_UTILITIES_FROM_PYGMO_HPP

#include <boost/any.hpp>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// Import and return the builtins module.
py::object builtins()
{
    return py::module::import("builtins");
}

// Throw a Python exception of type "type" with associated
// error message "msg".
void py_throw(PyObject *type, const char *msg)
{
    PyErr_SetString(type, msg);
    throw py::error_already_set();
}

// Generic extract() wrappers.
template <typename C, typename T>
inline T *generic_cpp_extract(C &c, const T &)
{
    return c.template extract<T>();
}

// Get the string representation of an object.
std::string str(const py::object &o)
{
    return py::cast<std::string>(py::str(o));
}

// Get the type of an object.
py::object type(const py::object &o)
{
    return builtins().attr("type")(o);
}

// Utils to expose algo log.
template <typename Algo>
inline py::list generic_log_getter(const Algo &a)
{
    py::list retval;
    for (const auto &t : a.get_log()) {
        retval.append(t);
    }
    return retval;
}

template <typename Algo>
inline void expose_algo_log(py::class_<Algo> &algo_class, const char *doc)
{
    algo_class.def("get_log", &generic_log_getter<Algo>, doc);
}

// Helper for the exposition of algorithms
// inheriting from not_population_based.
template <typename T>
inline void expose_not_population_based(py::class_<T> &c, const std::string &algo_name)
{
    // Selection/replacement.
    c.def_property(
        "selection",
        [](const T &n) -> py::object {
            auto s = n.get_selection();
            if (boost::any_cast<std::string>(&s)) {
                return py::str(boost::any_cast<std::string>(s));
            }
            return py::cast(boost::any_cast<pagmo::population::size_type>(s));
        },
        [](T &n, const py::object &o) {
            try {
                n.set_selection(py::cast<std::string>(o));
                return;
            } catch (const py::cast_error &) {
            }
            try {
                n.set_selection(py::cast<pagmo::population::size_type>(o));
                return;
            } catch (const py::cast_error &) {
            }
            py_throw(PyExc_TypeError,
                     ("cannot convert the input object '" + str(o) + "' of type '" + str(type(o))
                      + "' to either a selection policy (one of ['best', 'worst', 'random']) or an individual index")
                         .c_str());
        },
        ppnf::bls_selection_docstring(algo_name).c_str());
    c.def_property(
        "replacement",
        [](const T &n) -> py::object {
            auto s = n.get_replacement();
            if (boost::any_cast<std::string>(&s)) {
                return py::str(boost::any_cast<std::string>(s));
            }
            return py::cast(boost::any_cast<pagmo::population::size_type>(s));
        },
        [](T &n, const py::object &o) {
            try {
                n.set_replacement(py::cast<std::string>(o));
                return;
            } catch (const py::cast_error &) {
            }
            try {
                n.set_replacement(py::cast<pagmo::population::size_type>(o));
                return;
            } catch (const py::cast_error &) {
            }
            py_throw(PyExc_TypeError,
                     ("cannot convert the input object '" + str(o) + "' of type '" + str(type(o))
                      + "' to either a replacement policy (one of ['best', 'worst', 'random']) or an individual index")
                         .c_str());
        },
        ppnf::bls_replacement_docstring(algo_name).c_str());
    c.def("set_random_sr_seed", &T::set_random_sr_seed, ppnf::bls_set_random_sr_seed_docstring(algo_name).c_str());
}

#endif
