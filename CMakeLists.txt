cmake_minimum_required(VERSION 3.2)

project(pagmo-snopt7 VERSION 0.1)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules" "${CMAKE_SOURCE_DIR}/cmake_modules/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_INSTALL_PREFIX}/include")

# Set default build type to "Release".
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()

# Build option: enable test set.
option(PAGMO_SNOPT7_BUILD_TESTS "Build test set." OFF)

# Build option: enable python module.
option(PAGMO_SNOPT7_BUILD_PYTHON "Build the python module." OFF)

# Build option: install header.
option(PAGMO_SNOPT7_HEADERS "Enable the installation of the necessary's header files." ON)

# Initial setup of compiler flags.
include(YACMACompilerLinkerSettings)

# Threading setup.
include(YACMAThreadingSetup)

# Assemble the flags.
set(PAGMO_SNOPT7_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG} ${YACMA_THREADING_CXX_FLAGS})
set(PAGMO_SNOPT7_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS} ${YACMA_THREADING_CXX_FLAGS})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND YACMA_COMPILER_IS_CLANGXX)
  message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
  list(APPEND PAGMO_CXX_FLAGS_DEBUG "-stdlib=libc++")
  list(APPEND PAGMO_CXX_FLAGS_RELEASE "-stdlib=libc++")
endif()
if(YACMA_COMPILER_IS_MSVC)
  # Disable the idiotic minmax macros on MSVC, some annoying warnings,
  # and enable the bigobj option.
  list(APPEND PAGMO_SNOPT7_CXX_FLAGS_DEBUG "-DNOMINMAX" "/wd4459" "/wd4127" "/bigobj")
  list(APPEND PAGMO_SNOPT7_CXX_FLAGS_RELEASE "-DNOMINMAX" "/wd4459" "/wd4127" "/bigobj")
endif()
if(YACMA_COMPILER_IS_INTELXX)
  # NOTE: on MSVC we use the push/pop pragmas, but they do not seem to work on Intel (the pragmas
  # in icc influence the behaviour at instantiation point, not at definition point).
  # These warnings are useful in principle, but they are generated a lot from cereal and we have no
  # way of disabling them selectively. Just rely on the other compilers to provde good diagnostic.
  list(APPEND PAGMO_SNOPT7_CXX_FLAGS_DEBUG "-diag-disable" "2259,1682,68")
  list(APPEND PAGMO_SNOPT7_CXX_FLAGS_RELEASE "-diag-disable" "2259,1682,68")
endif()
if(MINGW)
	# Flag needed to deal with big binaries in MinGW.
	message(STATUS "Enabling the '-Wa,-mbig-obj' flag in MinGW builds.")
	list(APPEND PAGMO_SNOPT7_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
	list(APPEND PAGMO_SNOPT7_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

# Python setup.
# NOTE: we do it here because we need to detect the Python bits *before*
# looking for boost.python.
if(PAGMO_SNOPT7_BUILD_PYGMO)
    include(YACMAPythonSetup)

    # Python version check.
    if(${PYTHON_VERSION_MAJOR} LESS 2 OR (${PYTHON_VERSION_MAJOR} EQUAL 2 AND ${PYTHON_VERSION_MINOR} LESS 7))
        message(FATAL_ERROR "Minimum supported Python version is 2.7.")
    endif()

    if(NOT YACMA_NUMPY_INCLUDE_DIR)
        message(FATAL_ERROR "The NumPy headers could not be located.")
    endif()
endif()

# Boost setup.
include(PagmoFindBoost)

# Pagmo setup.
find_package(PAGMO REQUIRED)

# Setup of the header-only pagmo library.
add_library(pagmo_snopt7 INTERFACE)

target_include_directories(pagmo_snopt7 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(pagmo_snopt7 INTERFACE ${CMAKE_DL_LIBS} PAGMO::pagmo)

# Configure config.hpp.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/pagmo_snopt7/config.hpp" @ONLY)

# Configure the doc files.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/sphinx/conf.py" @ONLY)

if(PAGMO_SNOPT7_BUILD_TESTS)
    add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
endif()

if(PAGMO_SNOPT7_BUILD_PYTHON)
    add_subdirectory("${CMAKE_SOURCE_DIR}/pygmo")
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

if(PAGMO_SNOPT7_INSTALL_HEADERS)
  install(DIRECTORY include/ DESTINATION include)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/pagmo-snopt7/config.hpp" DESTINATION include/pagmo-snopt7)
endif()
