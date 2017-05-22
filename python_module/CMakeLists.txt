# Hard-coded.
include_directories(/home/dario/.local/include/pygmo)

YACMA_PYTHON_MODULE(pygmo_snopt7 core.cpp)

target_link_libraries(pygmo_snopt7 PRIVATE pagmo_snopt7 Boost::disable_autolinking Pagmo::pagmo)

add_library(pygmo SHARED IMPORTED)
set_target_properties(pygmo PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "/home/dario/.local/lib/python3.6/site-packages/pygmo/core.so")


target_link_libraries(pygmo_snopt7 PRIVATE pygmo)
target_include_directories(pygmo_snopt7 SYSTEM PRIVATE "${YACMA_NUMPY_INCLUDE_DIR}")
set_property(TARGET pygmo_snopt7 PROPERTY CXX_STANDARD 11)
set_property(TARGET pygmo_snopt7 PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET pygmo_snopt7 PROPERTY CXX_EXTENSIONS NO)

# Setup the installation path.
install(TARGETS pygmo_snopt7
RUNTIME DESTINATION "${YACMA_PYTHON_MODULES_INSTALL_PATH}"
LIBRARY DESTINATION "${YACMA_PYTHON_MODULES_INSTALL_PATH}"
)

