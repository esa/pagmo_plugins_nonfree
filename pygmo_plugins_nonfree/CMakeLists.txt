# Configure the version file.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_version.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/_version.py" @ONLY)   

# The list of pykep's Python files.
set(PPFN_PYTHON_FILES __init__.py _version.py test.py)

# Core module.
Python3_add_library(core MODULE WITH_SOABI 
    core.cpp
    docstrings.cpp
)

target_link_libraries(core PRIVATE pagmo_plugins_nonfree)
target_link_libraries(core PRIVATE "${pybind11_LIBRARIES}")

target_include_directories(core SYSTEM PRIVATE "${pybind11_INCLUDE_DIR}" "${Python3_INCLUDE_DIRS}")
target_compile_definitions(core PRIVATE "${pybind11_DEFINITIONS}")
target_compile_options(core PRIVATE
    "$<$<CONFIG:Debug>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE}>"
    
)
target_include_directories(core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(core PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(core PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
target_compile_features(core PRIVATE cxx_std_17)
set_property(TARGET core PROPERTY CXX_EXTENSIONS NO)

# Installation setup.
if(PPFN_INSTALL_PATH STREQUAL "")
    message(STATUS "pygmo_plugins_nonfree will be installed in the default location: ${Python3_SITEARCH}")
    set(_PPNF_INSTALL_PATH "${Python3_SITEARCH}/pygmo_plugins_nonfree")
else()
    message(STATUS "pygmo_plugins_nonfree will be installed in the custom location: ${PPFN_INSTALL_PATH}")
    set(_PPNF_INSTALL_PATH "${PPFN_INSTALL_PATH}/pygmo_plugins_nonfree")
endif()

# Install the core module.
install(TARGETS core
    RUNTIME DESTINATION ${_PPNF_INSTALL_PATH}
    LIBRARY DESTINATION ${_PPNF_INSTALL_PATH}
)

# Install the Python files.
install(FILES ${PPFN_PYTHON_FILES} DESTINATION ${_PPNF_INSTALL_PATH})

unset(_PPNF_INSTALL_PATH)