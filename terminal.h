//
// Created by gustavo on 21/06/2026.
//

#ifndef TRAB2_OC_NOC_SPIN_TERMINAL_H
#define TRAB2_OC_NOC_SPIN_TERMINAL_H
#include <systemc.h>
#include "spin_flit.h"

SC_MODULE(Terminal) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    sc_out<SpinFlit> tx;
    sc_in<SpinFlit>  rx;

    sc_in<bool>  credit_in;
    sc_out<bool> credit_out;

    int my_id;
    int target_id;
    int flits_to_send;
    int my_credits;
    
    void set_target(int t_id);
    void process_traffic();

    SC_HAS_PROCESS(Terminal);
    Terminal(sc_module_name name, int id);
};
#endif //TRAB2_OC_NOC_SPIN_TERMINAL_H
