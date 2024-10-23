# NOTE: current minimum requirement.
cmake_minimum_required(VERSION 3.18.0)

# Module path setup.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules" "${CMAKE_SOURCE_DIR}/cmake_modules/yacma")

# Set default build type to "Release".
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()

# Main build options: build pagmo or pygmo. They cannot be on at the same time,
# and only one must be chosen.
option(PPNF_BUILD_CPP "Build the C++ code." ON)
option(PPNF_BUILD_PYTHON "Builds the python module (requires the C++ code to be installed first)." OFF)

# Check consistency.
if(PPNF_BUILD_CPP AND PPNF_BUILD_PYTHON)
    message(FATAL_ERROR "Please select whether to build the C++ or the Python code: you cannot build them both at the same time.")
endif()

if((NOT PPNF_BUILD_CPP) AND (NOT PPNF_BUILD_PYTHON))
    message(FATAL_ERROR "Please select if you want to build the C++ or the Python code.")
endif()

# Main ppnf project version.
set(pagmo_plugins_nonfree_VERSION 0.26)

if(PPNF_BUILD_CPP)
    # Initial setup of a pagmo_plugins_nonfree build.
    project(pagmo_plugins_nonfree VERSION ${pagmo_plugins_nonfree_VERSION} LANGUAGES CXX C)
    message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
    enable_testing()
    # Build option: enable test set.
    option(PPNF_BUILD_TESTS "Build test set." OFF)
else()
    # Initial setup of a pygmo_plugins_nonfree build.
    project(pygmo_plugins_nonfree VERSION ${pagmo_plugins_nonfree_VERSION} LANGUAGES CXX C)
    message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
endif()

# Common general bits.

# Initial setup of compiler flags.
include(YACMACompilerLinkerSettings)

# Assemble the flags.
set(PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG} ${YACMA_THREADING_CXX_FLAGS})
set(PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS} ${YACMA_THREADING_CXX_FLAGS})
if(APPLE AND YACMA_COMPILER_IS_CLANGXX)
  message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-stdlib=libc++")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-stdlib=libc++")
endif()
if(YACMA_COMPILER_IS_MSVC)
  # Disable the idiotic minmax macros on MSVC, some annoying warnings,
  # and enable the bigobj option.
  list(APPEND PAGMO_PLUGINS_NONFREECXX_FLAGS_DEBUG "-DNOMINMAX" "/wd4459" "/wd4127" "/wd4702" "/wd4251" "/bigobj")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-DNOMINMAX" "/wd4459" "/wd4127" "/wd4702" "/wd4251" "/bigobj")
  # On Windows systems and pybind11 this may be needed to avoid this issue (https://github.com/boostorg/system/issues/32)
  add_definitions(-DHAVE_SNPRINTF)
endif()
if(MINGW)
	# Flag needed to deal with big binaries in MinGW.
	message(STATUS "Enabling the '-Wa,-mbig-obj' flag in MinGW builds.")
	list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
	list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

# Setting the boost libraries needed for PPNF
# NOTE: need 1.48 as pagmo.
set(_PPNF_MIN_BOOST_VERSION "1.48")
find_package(Boost ${_PPNF_MIN_BOOST_VERSION} REQUIRED COMPONENTS system filesystem unit_test_framework serialization)
MESSAGE(STATUS "Detected Boost version: ${Boost_VERSION}")
# Include system Boost headers.
MESSAGE(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
MESSAGE(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# Pagmo setup (common to pagmo/pygmo . pluginsnonfree).
find_package(Pagmo REQUIRED)

if(PPNF_BUILD_CPP)
    # List of source files.
    set(PAGMO_PLUGINS_NONFREE_SRC_FILES
        # Core classes.
        "${CMAKE_CURRENT_SOURCE_DIR}/src/snopt7.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/worhp.cpp"
    )

    # Setup of the pagmo library.
    add_library(pagmo_plugins_nonfree SHARED "${PAGMO_PLUGINS_NONFREE_SRC_FILES}")
    set_property(TARGET pagmo_plugins_nonfree PROPERTY VERSION "1.0")
    set_property(TARGET pagmo_plugins_nonfree PROPERTY SOVERSION 1)
    target_compile_options(pagmo_plugins_nonfree PRIVATE "$<$<CONFIG:DEBUG>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG}>" "$<$<CONFIG:RELEASE>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE}>")
    set_target_properties(pagmo_plugins_nonfree PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(pagmo_plugins_nonfree PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
    # Let's setup the target C++ standard, but only if the user did not provide it manually.
    if(NOT CMAKE_CXX_STANDARD)
        set_property(TARGET pagmo_plugins_nonfree PROPERTY CXX_STANDARD 17)
    endif()
    set_property(TARGET pagmo_plugins_nonfree PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET pagmo_plugins_nonfree PROPERTY CXX_EXTENSIONS NO)
    # NOTE: make sure the include directories from the current build
    # are included first, so that if there is already a pagmo_plugins_nonfree installation
    # in the prefix path we don't risk including the headers from that
    # one instead.
    target_include_directories(pagmo_plugins_nonfree PUBLIC
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
       $<INSTALL_INTERFACE:include>)

    # Boost.
    target_link_libraries(pagmo_plugins_nonfree PUBLIC Boost::boost Boost::serialization Boost::disable_autolinking Boost::system Boost::filesystem)

    # Pagmo.
    target_link_libraries(pagmo_plugins_nonfree PUBLIC Pagmo::pagmo)

    # DL libraries
    target_link_libraries(pagmo_plugins_nonfree PUBLIC ${CMAKE_DL_LIBS})

    # Configure config.hpp.
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/pagmo_plugins_nonfree/config.hpp" @ONLY)

    # Configure the doc files.
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py" @ONLY)

    # Build the tests
    if(PPNF_BUILD_TESTS)
        add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
    endif()
endif()

if(PPNF_BUILD_PYTHON)
    # Find python
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
    message(STATUS "Python3 interpreter: ${Python3_EXECUTABLE}")
    message(STATUS "Python3 installation directory: ${Python3_SITEARCH}")
    message(STATUS "Python3 include directories: ${Python3_INCLUDE_DIRS}")

    set(PPFN_INSTALL_PATH "" CACHE STRING "ppfn module installation path")
    mark_as_advanced(PPFN_INSTALL_PATH)

    # pybind11.
    find_package(pybind11 REQUIRED)

    # Build directory
    add_subdirectory("${CMAKE_SOURCE_DIR}/pygmo_plugins_nonfree")

    if(MINGW OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        message(STATUS "Creating the files for the generation of a binary wheel.")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/wheel_setup.py" "${CMAKE_CURRENT_BINARY_DIR}/wheel/setup.py" @ONLY)
        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            # NOTE: this is necessary on linux but harmful on mingw.
            configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/wheel_setup.cfg" "${CMAKE_CURRENT_BINARY_DIR}/wheel/setup.cfg" @ONLY)
        endif()
        if(MINGW)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/mingw_wheel_libs.txt" "${CMAKE_CURRENT_BINARY_DIR}/wheel/mingw_wheel_libs.txt" @ONLY)
        endif()
    endif()
endif()


# Library installation.
if(PPNF_BUILD_CPP)
    # Installation of the header files.
    install(DIRECTORY include/ DESTINATION include)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/pagmo_plugins_nonfree/config.hpp" DESTINATION include/pagmo_plugins_nonfree)

    # Installation of the library.
    install(TARGETS pagmo_plugins_nonfree
        EXPORT pagmo_plugins_nonfree_export
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pagmo_plugins_nonfree-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/pagmo_plugins_nonfree-config.cmake" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pagmo_plugins_nonfree-config.cmake" DESTINATION "lib/cmake/pagmo_plugins_nonfree")
    install(EXPORT pagmo_plugins_nonfree_export NAMESPACE Pagmo_plugins_nonfree:: DESTINATION lib/cmake/pagmo_plugins_nonfree)
    # Take care of versioning.
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/pagmo_plugins_nonfree-config-version.cmake" VERSION ${pagmo_plugins_nonfree_VERSION}
        COMPATIBILITY ExactVersion)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pagmo_plugins_nonfree-config-version.cmake" DESTINATION "lib/cmake/pagmo_plugins_nonfree")
endif()

# Uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
