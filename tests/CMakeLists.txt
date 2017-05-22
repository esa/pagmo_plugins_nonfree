# For testing we build a fake library that we will call snopt7_c
# and that does nothing.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/snopt7_c_lib)
add_library(snopt7_c SHARED ../snopt7_c_lib/snopt7_c.c)
install(DIRECTORY include/ DESTINATION include)

function(ADD_PAGMO_SNOPT7_TESTCASE arg1)
    add_executable(${arg1} ${arg1}.cpp)
    target_link_libraries(${arg1} pagmo_snopt7)
    target_compile_options(${arg1} PRIVATE "$<$<CONFIG:DEBUG>:${PAGMO_SNOPT7_CXX_FLAGS_DEBUG}>" "$<$<CONFIG:RELEASE>:${PAGMO_SNOPT7_CXX_FLAGS_RELEASE}>")
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
    add_test(${arg1} ${arg1})
endfunction()

# Tests requiring no dependencies (in alphabetical order)
ADD_PAGMO_SNOPT7_TESTCASE(snopt7)
