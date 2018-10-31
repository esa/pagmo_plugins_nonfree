cmake_minimum_required(VERSION 3.2)

set(PAGMO_PLUGINS_NONFREE_PROJECT_VERSION 0.5)
project(pagmo_plugins_nonfree VERSION ${PAGMO_PLUGINS_NONFREE_PROJECT_VERSION})

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules" "${CMAKE_SOURCE_DIR}/cmake_modules/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")

# Set default build type to "Release".
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()

# Build option: enable test set.
option(PAGMO_PLUGINS_NONFREE_BUILD_TESTS "Build test set." OFF)

# Build option: enable python module.
option(PAGMO_PLUGINS_NONFREE_BUILD_PYTHON "Build the python module." OFF)

# Build option: install header.
option(PAGMO_PLUGINS_NONFREE_HEADERS "Enable the installation of the necessary header files." ON)

# Initial setup of compiler flags.
include(YACMACompilerLinkerSettings)

# Assemble the flags.
set(PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG})
set(PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND YACMA_COMPILER_IS_CLANGXX)
  message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-stdlib=libc++")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-stdlib=libc++")
endif()
if(YACMA_COMPILER_IS_MSVC)
  # Disable the idiotic minmax macros on MSVC, some annoying warnings,
  # and enable the bigobj option.
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-DNOMINMAX" "/wd4459" "/wd4127" "/bigobj")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-DNOMINMAX" "/wd4459" "/wd4127" "/bigobj")
endif()
if(YACMA_COMPILER_IS_INTELXX)
  # NOTE: on MSVC we use the push/pop pragmas, but they do not seem to work on Intel (the pragmas
  # in icc influence the behaviour at instantiation point, not at definition point).
  # These warnings are useful in principle, but they are generated a lot from cereal and we have no
  # way of disabling them selectively. Just rely on the other compilers to provde good diagnostic.
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-diag-disable" "2259,1682,68")
  list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-diag-disable" "2259,1682,68")
endif()
if(MINGW)
	# Flag needed to deal with big binaries in MinGW.
	message(STATUS "Enabling the '-Wa,-mbig-obj' flag in MinGW builds.")
	list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
	list(APPEND PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

if(PAGMO_PLUGINS_NONFREE_BUILD_PYTHON)
    # pygmo_pnf dependencies.
    include(YACMAPythonSetup)

    # Python version check.
    if(${PYTHON_VERSION_MAJOR} LESS 2 OR (${PYTHON_VERSION_MAJOR} EQUAL 2 AND ${PYTHON_VERSION_MINOR} LESS 7))
        message(FATAL_ERROR "Minimum supported Python version is 2.7.")
    endif()

    # Internal variable that will be used to tell PagmoFindBoost to locate Boost.Python.
    set(_PAGMO_PLUGINS_NONFREE_FIND_BOOST_PYTHON TRUE)
endif()

# Boost setup (common to pagmo/pygmo).
include(PagmoPNFFindBoost)

# Pagmo setup.
message(STATUS "\nFinding the pagmo package:")
find_package(Pagmo REQUIRED)
message(STATUS "End of finding the pagmo package\n")

# Setup of the header-only pagmo_plugins_nonfree library.
add_library(pagmo_plugins_nonfree INTERFACE)

target_include_directories(pagmo_plugins_nonfree INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(pagmo_plugins_nonfree INTERFACE Boost::disable_autolinking Boost::system Boost::filesystem Pagmo::pagmo ${CMAKE_DL_LIBS})

# Configure config.hpp.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/pagmo_plugins_nonfree/config.hpp" @ONLY)

# Configure the doc files.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py" @ONLY)

# Build the tests
if(PAGMO_PLUGINS_NONFREE_BUILD_TESTS)
    add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
endif()

# Build the pygmo_plugins_nonfree module
if(PAGMO_PLUGINS_NONFREE_BUILD_PYTHON)
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

if(PAGMO_PLUGINS_NONFREE_HEADERS)
  install(DIRECTORY include/ DESTINATION include)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/pagmo_plugins_nonfree/config.hpp" DESTINATION include/pagmo_plugins_nonfree)
endif()
