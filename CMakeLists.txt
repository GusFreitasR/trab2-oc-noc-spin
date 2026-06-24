cmake_minimum_required(VERSION 3.10)
project(trab2_oc_noc_spin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Esconde os avisos de funções obsoletas do SystemC 3.0.2
add_compile_definitions(SC_ALLOW_DEPRECATED_IEEE_API)

set(SYSTEMC_HOME "/usr/local/systemc-3.0.2")

add_executable(trab2_oc_noc_spin
        main.cpp
        spin_flit.h
        terminal.h
        rspin_router.h
        terminal.cpp
        rspin_router.cpp
        spin_fat_tree.h
        spin_fat_tree.cpp)

target_include_directories(trab2_oc_noc_spin PRIVATE .)
target_include_directories(trab2_oc_noc_spin PRIVATE ${SYSTEMC_HOME}/include)

# 1. Caminho correto confirmado pelo seu comando ls
target_link_directories(trab2_oc_noc_spin PRIVATE ${SYSTEMC_HOME}/lib-linux64)

# 2. Linka a biblioteca SystemC
target_link_libraries(trab2_oc_noc_spin systemc)

# 3. Embutir o caminho no executável para não dar erro de .so ao rodar
set_target_properties(trab2_oc_noc_spin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
set_target_properties(trab2_oc_noc_spin PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(trab2_oc_noc_spin PROPERTIES INSTALL_RPATH "${SYSTEMC_HOME}/lib-linux64")