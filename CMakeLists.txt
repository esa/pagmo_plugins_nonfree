# CMake version check.
cmake_minimum_required(VERSION 3.2)

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
set(PAGMO_PLUGINS_NONFREE_VERSION 0.10)

if(PPNF_BUILD_CPP)
    # Initial setup of a pagmo_plugins_nonfree build.
    project(pagmo_plugins_nonfree VERSION ${PAGMO_PLUGINS_NONFREE_VERSION} LANGUAGES CXX C)
    message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
    enable_testing()
    # Build option: enable test set.
    option(PPNF_BUILD_TESTS "Build test set." OFF)
else()
    # Initial setup of a pygmo_plugins_nonfree build.
    project(pygmo_plugins_nonfree VERSION ${PAGMO_PLUGINS_NONFREE_VERSION} LANGUAGES CXX C)
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
endif()
if(MINGW)
	# Flag needed to deal with big binaries in MinGW.
	message(STATUS "Enabling the '-Wa,-mbig-obj' flag in MinGW builds.")
	list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
	list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

if(PPNF_BUILD_CPP)
    # pagmo plugins non free dependencies
    if(PPNF_BUILD_TESTS)
        # Internal variable that will be used to tell PagmoPNFFindBoost to locate the
        # Boost unit test framework, if tests are required.
        set(_PAGMO_PLUGINS_NONFREE_FIND_BOOST_UNIT_TEST_FRAMEWORK TRUE)
    endif()
endif()

if(PPNF_BUILD_PYTHON)
    # pygmo plugins non free dependencies
    include(YACMAPythonSetup)

    # Python version check.
    if(${PYTHON_VERSION_MAJOR} LESS 2 OR (${PYTHON_VERSION_MAJOR} EQUAL 2 AND ${PYTHON_VERSION_MINOR} LESS 7))
        message(FATAL_ERROR "Minimum supported Python version is 2.7.")
    endif()

    # Internal variable that will be used to tell PagmoFindBoost to locate Boost.Python.
    set(_PAGMO_PLUGINS_NONFREE_FIND_BOOST_PYTHON TRUE)
endif()


# Boost setup (common to pagmo/pygmo . pluginsnonfree).
include(PagmoPNFFindBoost)
# Pagmo setup (common to pagmo/pygmo . pluginsnonfree).
find_package(Pagmo 2.11.3 REQUIRED)

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
        set_property(TARGET pagmo_plugins_nonfree PROPERTY CXX_STANDARD 11)
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

# Build the pygmo_plugins_nonfree module
if(PPNF_BUILD_PYTHON)
    add_subdirectory("${CMAKE_SOURCE_DIR}/pygmo_plugins_nonfree")
    if(MINGW OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        message(STATUS "Creating the files for the generation of a binary wheel.")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/wheel_setup.py" "${CMAKE_CURRENT_BINARY_DIR}/wheel/setup.py" @ONLY)
        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            # NOTE: this is necessary on linux but harmful on mingw.
            configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/wheel_setup.cfg" "${CMAKE_CURRENT_BINARY_DIR}/wheel/setup.cfg" @ONLY)
        endif()
        if(MINGW)
            configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/mingw_wheel_libs_python${PYTHON_VERSION_MAJOR}.txt" "${CMAKE_CURRENT_BINARY_DIR}/wheel/mingw_wheel_libs_python${PYTHON_VERSION_MAJOR}.txt" @ONLY)
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
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/PagmoPNFFindBoost.cmake" DESTINATION "lib/cmake/pagmo_plugins_nonfree")
    install(EXPORT pagmo_plugins_nonfree_export NAMESPACE Pagmo_plugins_nonfree:: DESTINATION lib/cmake/pagmo_plugins_nonfree)
    # Take care of versioning.
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/pagmo_plugins_nonfree-config-version.cmake" VERSION ${PAGMO_PLUGINS_NONFREE_VERSION}
        COMPATIBILITY ExactVersion)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pagmo_plugins_nonfree-config-version.cmake" DESTINATION "lib/cmake/pagmo_plugins_nonfree")
endif()
