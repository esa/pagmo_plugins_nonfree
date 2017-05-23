# Hard-coded.
include_directories(${CMAKE_INSTALL_PREFIX}/include/pygmo)

YACMA_PYTHON_MODULE(core core.cpp)

target_link_libraries(core PRIVATE pagmo_snopt7 Boost::disable_autolinking Boost::python Pagmo::pagmo)

target_include_directories(core SYSTEM PRIVATE "${YACMA_NUMPY_INCLUDE_DIR}")
set_property(TARGET core PROPERTY CXX_STANDARD 11)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)

# Helper file with version number.
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in" "${CMAKE_CURRENT_BINARY_DIR}/_version.py" @ONLY)

# Setup the installation path.
set(PYGMO_SNOPT7_INSTALL_PATH "${YACMA_PYTHON_MODULES_INSTALL_PATH}/pygmo_snopt7")
install(TARGETS core
 RUNTIME DESTINATION ${PYGMO_SNOPT7_INSTALL_PATH}
 LIBRARY DESTINATION ${PYGMO_SNOPT7_INSTALL_PATH}
)

# Add the Python files.
install(FILES __init__.py "${CMAKE_CURRENT_BINARY_DIR}/_version.py"
     DESTINATION ${PYGMO_SNOPT7_INSTALL_PATH})