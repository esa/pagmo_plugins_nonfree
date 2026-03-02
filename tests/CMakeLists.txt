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
    # On macOS, link against the static library to avoid Boost.Serialization
    # void_cast lookup failures caused by macOS two-level namespace: type_info::before()
    # uses pointer identity, so identical template type_info objects in different DSOs
    # (shared lib vs. test binary) have different addresses and are treated as separate
    # types by void_cast_registry. Static linking puts all code in one binary with a
    # single consistent set of type_info objects.
    if(APPLE)
        target_link_libraries(${arg1} pagmo_plugins_nonfree_static Boost::unit_test_framework)
    else()
        target_link_libraries(${arg1} pagmo_plugins_nonfree Boost::unit_test_framework)
    endif()
    target_compile_options(${arg1} PRIVATE "$<$<CONFIG:DEBUG>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_DEBUG}>" "$<$<CONFIG:RELEASE>:${PAGMO_PLUGINS_NONFREE_CXX_FLAGS_RELEASE}>")
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
    add_test(${arg1} ${arg1})
endfunction()

# Tests
ADD_PAGMO_PLUGINS_TESTCASE(snopt7)
ADD_PAGMO_PLUGINS_TESTCASE(worhp)

