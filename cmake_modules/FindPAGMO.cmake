if(PAGMO_INCLUDE_DIR)
	# Already in cache, be silent
	set(PAGMO_FIND_QUIETLY TRUE)
endif()

find_path(PAGMO_INCLUDE_DIR NAMES pagmo.hpp PATH_SUFFIXES pagmo)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PAGMO DEFAULT_MSG PAGMO_INCLUDE_DIR)

mark_as_advanced(PAGMO_INCLUDE_DIR)

# NOTE: this has been adapted from CMake's FindPNG.cmake.
if(PAGMO_FOUND AND NOT TARGET PAGMO::pagmo)
    add_library(PAGMO::pagmo UNKNOWN IMPORTED)
    set_target_properties(PAGMO::pagmo PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PAGMO_INCLUDE_DIR}")
endif()