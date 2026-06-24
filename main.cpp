#include <systemc.h>
#include "spin_fat_tree.h"

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;

    Spin_Fat_Tree my_noc("SPIN_FatTree_RTL_Wormhole");
    my_noc.clk(clk);
    my_noc.rst(rst);

    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << " Iniciando Simulacao WORMHOLE + CREDITOS (Arquitetura SPIN)" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    sc_start(0, SC_NS);
    rst.write(true);
    sc_start(20, SC_NS);
    rst.write(false);
    sc_start(10, SC_NS);

    my_noc.configure_traffic(1, 14);

    sc_start(300, SC_NS);

    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "                  Simulacao Concluida.                  " << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    return 0;
}