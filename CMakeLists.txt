cmake_minimum_required(VERSION 3.16)
project(spin_noc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(SC_ALLOW_DEPRECATED_IEEE_API)

if(NOT DEFINED SYSTEMC_HOME)
    if(DEFINED ENV{SYSTEMC_HOME})
        set(SYSTEMC_HOME $ENV{SYSTEMC_HOME})
    elseif(EXISTS "$ENV{HOME}/.local/systemc-3.0.2")
        set(SYSTEMC_HOME "$ENV{HOME}/.local/systemc-3.0.2")
    else()
        set(SYSTEMC_HOME "/usr/local/systemc-3.0.2")
    endif()
endif()

find_library(SYSTEMC_LIBRARY
    NAMES systemc
    PATHS ${SYSTEMC_HOME}/lib ${SYSTEMC_HOME}/lib64 ${SYSTEMC_HOME}/lib-linux64
    REQUIRED)
get_filename_component(SYSTEMC_LIBDIR ${SYSTEMC_LIBRARY} DIRECTORY)

add_library(spin_noc STATIC
    src/trace.cpp
    src/terminal.cpp
    src/router.cpp
    src/fat_tree.cpp)
target_include_directories(spin_noc PUBLIC include ${SYSTEMC_HOME}/include)
target_link_libraries(spin_noc PUBLIC ${SYSTEMC_LIBRARY})

enable_testing()
set(SPIN_TESTS
    test_single_flow
    test_local_flow
    test_convergence
    test_permutation
    test_many_to_one)

foreach(test_name ${SPIN_TESTS})
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE spin_noc)
    set_target_properties(${test_name} PROPERTIES BUILD_RPATH ${SYSTEMC_LIBDIR})
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "SYSTEMC_DISABLE_COPYRIGHT_MESSAGE=1")
endforeach()
