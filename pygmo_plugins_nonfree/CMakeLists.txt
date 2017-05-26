YACMA_PYTHON_MODULE(core core.cpp docstrings.cpp)
find_package(Pygmo)
target_link_libraries(core PRIVATE pagmo_plugins_nonfree Pygmo::pygmo)

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
install(FILES __init__.py "${CMAKE_CURRENT_BINARY_DIR}/_version.py"
     DESTINATION ${PYGMO_PLUGINS_NONFREE_INSTALL_PATH})