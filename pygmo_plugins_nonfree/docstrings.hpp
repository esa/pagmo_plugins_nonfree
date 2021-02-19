#ifndef PYGMO_PLUGINS_NONFREE_DOCSTRINGS_HPP
#define PYGMO_PLUGINS_NONFREE_DOCSTRINGS_HPP

#include <string>

namespace ppnf
{
// not_population_based.
std::string bls_selection_docstring(const std::string &);
std::string bls_replacement_docstring(const std::string &);
std::string bls_set_random_sr_seed_docstring(const std::string &);
// snopt7
std::string snopt7_docstring();
std::string snopt7_get_log_docstring();
std::string snopt7_set_integer_option_docstring();
std::string snopt7_set_numeric_option_docstring();
// worhp
std::string worhp_docstring();
std::string worhp_get_log_docstring();
std::string worhp_set_integer_option_docstring();
std::string worhp_set_numeric_option_docstring();
std::string worhp_set_bool_option_docstring();
std::string worhp_zen_update_docstring();
}

#endif
