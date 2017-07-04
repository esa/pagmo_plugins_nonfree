# Python version check.
if(${PYTHON_VERSION_MAJOR} LESS 2 OR (${PYTHON_VERSION_MAJOR} EQUAL 2 AND ${PYTHON_VERSION_MINOR} LESS 7))
    message(FATAL_ERROR "Minimum supported Python version is 2.7.")
endif()

# Finding pygmo
find_package(Pygmo REQUIRED)

# Setting the rpath
if(UNIX)
    # On unix platforms, we set the rpath of the pygmo libraries
    # to the lib subdirectory of the install prefix. This allows
    # to install pygmo in nonstandard locations (e.g., ~/.local)
    # without having to muck around with library search paths.
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# The name of the BP target to link to depends on the Python version.
if(${PYTHON_VERSION_MAJOR} EQUAL 2)
    set(PYGMO_PLUGINS_NONFREE_BP_TARGET "Boost::python")
else()
    set(PYGMO_PLUGINS_NONFREE_BP_TARGET "Boost::python3")
endif()

# Create the module
YACMA_PYTHON_MODULE(core core.cpp docstrings.cpp)
target_link_libraries(core PRIVATE pagmo_plugins_nonfree ${PYGMO_PLUGINS_NONFREE_BP_TARGET} Boost::disable_autolinking Pygmo::pygmo)
set_property(TARGET core PROPERTY CXX_STANDARD 11)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)

# Helper file with version number.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in" "${CMAKE_CURRENT_BINARY_DIR}/_version.py" @ONLY)

# Setup the installation path.
set(PYGMO_PLUGINS_NONFREE_INSTALL_PATH "${YACMA_PYTHON_MODULES_INSTALL_PATH}/pygmo_plugins_nonfree")
install(TARGETS core
 RUNTIME DESTINATION ${PYGMO_PLUGINS_NONFREE_INSTALL_PATH}
 LIBRARY DESTINATION ${PYGMO_PLUGINS_NONFREE_INSTALL_PATH}
)

# Add the Python files.
install(FILES __init__.py test.py "${CMAKE_CURRENT_BINARY_DIR}/_version.py"
     DESTINATION ${PYGMO_PLUGINS_NONFREE_INSTALL_PATH})