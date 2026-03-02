# For testing we build a fake library that we will call snopt7_c
# and that does nothing useful.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/pagmo_plugins_nonfree/bogus_libs/snopt7_c_lib)
add_library(snopt7_c SHARED ../include/pagmo_plugins_nonfree/bogus_libs/snopt7_c_lib/snopt7_c.c)

# For testing we build a fake library that we will call worhp
# and that does nothing useful.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/pagmo_plugins_nonfree/bogus_libs/worhp_lib)
add_library(worhp_c SHARED ../include/pagmo_plugins_nonfree/bogus_libs/worhp_lib/worhp_bogus.c)

function(ADD_PAGMO_PLUGINS_TESTCASE arg1)
    add_executable(${arg1} ${arg1}.cpp)
    target_link_libraries(${arg1} pagmo_plugins_nonfree Boost::unit_test_framework)
    target_compile_options(${arg1} PRIVATE "$<$<CONFIG:DEBUG>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG}>" "$<$<CONFIG:RELEASE>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE}>")
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
    add_test(${arg1} ${arg1})
    # On macOS, std::type_info::before() uses pointer identity for ordering.
    # With two-level namespace, different DSOs have different std::type_info
    # pointer values for the same type, causing Boost.Serialization's
    # void_cast_registry (an ordered set keyed by type_info::before) to fail
    # to find void_casts that were registered from a different DSO.
    # DYLD_FORCE_FLAT_NAMESPACE=1 coalesces all symbol copies across DSOs so
    # every image uses the same type_info objects and the same singletons.
    if(APPLE)
        set_tests_properties(${arg1} PROPERTIES
            ENVIRONMENT "DYLD_FORCE_FLAT_NAMESPACE=1")
    endif()
endfunction()

# Tests
ADD_PAGMO_PLUGINS_TESTCASE(snopt7)
ADD_PAGMO_PLUGINS_TESTCASE(worhp)

