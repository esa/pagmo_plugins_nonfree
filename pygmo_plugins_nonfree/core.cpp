#include <boost/numeric/conversion/cast.hpp>
#include <iostream>
#include <pagmo/s11n.hpp>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <sstream>
#include <string>

#include <pagmo_plugins_nonfree/snopt7.hpp>
#include <pagmo_plugins_nonfree/worhp.hpp>

#include "docstrings.hpp"
#include "utilities_from_pygmo.hpp"

namespace py = pybind11;

// Serialization support
template <typename UDA>
py::tuple uda_pickle_getstate(const UDA &uda)
{
    // The idea here is that first we extract a char array
    // into which a has been serialized, then we turn
    // this object into a Python bytes object and return that.
    std::ostringstream oss;
    {
        boost::archive::binary_oarchive oarchive(oss);
        oarchive << uda;
    }
    auto s = oss.str();
    return py::make_tuple(py::bytes(s.data(), boost::numeric_cast<py::size_t>(s.size())));
}

template <typename UDA>
UDA uda_pickle_setstate(py::tuple state)
{
    // Similarly, first we extract a bytes object from the Python state,
    // and then we build a C++ string from it. The string is then used
    // to deserialized the object.
    if (py::len(state) != 1) {
        py_throw(PyExc_ValueError, ("the state tuple passed for uda deserialization "
                                    "must have 1 element, but instead it has "
                                    + std::to_string(py::len(state)) + " element(s)")
                                       .c_str());
    }

    auto ptr = PyBytes_AsString(state[0].ptr());
    if (!ptr) {
        py_throw(PyExc_TypeError, "a bytes object is needed to deserialize an algorithm");
    }

    std::istringstream iss;
    iss.str(std::string(ptr, ptr + py::len(state[0])));
    UDA uda;
    {
        boost::archive::binary_iarchive iarchive(iss);
        iarchive >> uda;
    }

    return uda;
}

PYBIND11_MODULE(core, m)
{
    // This function needs to be called before doing anything with threads.
    // https://docs.python.org/3/c-api/init.html
    PyEval_InitThreads();

    // Disable automatic function signatures in the docs.
    // NOTE: the 'options' object needs to stay alive
    // throughout the whole definition of the module.
    py::options options;
    options.disable_function_signatures();

    // snopt7
    py::class_<ppnf::snopt7> snopt7_(m, "snopt7", ppnf::snopt7_docstring().c_str());
    snopt7_.def(py::init<>());
    // We expose the additional constructor
    snopt7_.def(py::init<bool, std::string, unsigned>(), py::arg("screen_output") = false,
                py::arg("library") = "/usr/local/lib/", py::arg("minor_version") = 6);
    snopt7_.def("evolve", &ppnf::snopt7::evolve);
    snopt7_.def("set_verbosity", &ppnf::snopt7::set_verbosity);
    snopt7_.def("get_name", &ppnf::snopt7::get_name);
    snopt7_.def("get_extra_info", &ppnf::snopt7::get_extra_info);
    snopt7_.def("set_integer_option", &ppnf::snopt7::set_integer_option,
                ppnf::snopt7_set_integer_option_docstring().c_str(), py::arg("name"), py::arg("value"));
    snopt7_.def("set_numeric_option", &ppnf::snopt7::set_numeric_option,
                ppnf::snopt7_set_numeric_option_docstring().c_str(), py::arg("name"), py::arg("value"));
    snopt7_.def(py::pickle(&uda_pickle_getstate<ppnf::snopt7>, &uda_pickle_setstate<ppnf::snopt7>));
    expose_algo_log(snopt7_, ppnf::snopt7_get_log_docstring().c_str());
    expose_not_population_based(snopt7_, "snopt7");

    py::class_<ppnf::worhp> worhp_(m, "worhp", ppnf::worhp_docstring().c_str());
    worhp_.def(py::init<>());
    // We expose the additional constructor
    worhp_.def(py::init<bool, std::string>(), py::arg("screen_output") = false, py::arg("library") = "/usr/local/lib/");
    worhp_.def("evolve", &ppnf::worhp::evolve);
    worhp_.def("set_verbosity", &ppnf::worhp::set_verbosity);
    worhp_.def("get_name", &ppnf::worhp::get_name);
    worhp_.def("get_extra_info", &ppnf::worhp::get_extra_info);
    worhp_.def("set_integer_option", &ppnf::worhp::set_integer_option,
               ppnf::worhp_set_integer_option_docstring().c_str(), py::arg("name"), py::arg("value"));
    worhp_.def("set_numeric_option", &ppnf::worhp::set_numeric_option,
               ppnf::worhp_set_numeric_option_docstring().c_str(), py::arg("name"), py::arg("value"));
    worhp_.def("set_bool_option", &ppnf::worhp::set_bool_option, ppnf::worhp_set_bool_option_docstring().c_str(),
               py::arg("name"), py::arg("value"));
    worhp_.def(py::pickle(&uda_pickle_getstate<ppnf::worhp>, &uda_pickle_setstate<ppnf::worhp>));
    expose_algo_log(worhp_, ppnf::worhp_get_log_docstring().c_str());
    expose_not_population_based(worhp_, "worhp");
}
