# Hard-coded.
include_directories(/home/dario/.local/include/pygmo)

YACMA_PYTHON_MODULE(core core.cpp)

target_link_libraries(core PRIVATE pagmo_snopt7 Boost::disable_autolinking Boost::python Pagmo::pagmo)

add_library(pygmo SHARED IMPORTED)
set_target_properties(pygmo PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "/home/dario/.local/lib/python3.6/site-packages/pygmo/core.so")


target_link_libraries(core PRIVATE pygmo)
target_include_directories(core SYSTEM PRIVATE "${YACMA_NUMPY_INCLUDE_DIR}")
set_property(TARGET core PROPERTY CXX_STANDARD 11)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)

# Setup the installation path.
install(TARGETS core
RUNTIME DESTINATION "${YACMA_PYTHON_MODULES_INSTALL_PATH}"
LIBRARY DESTINATION "${YACMA_PYTHON_MODULES_INSTALL_PATH}"
)

