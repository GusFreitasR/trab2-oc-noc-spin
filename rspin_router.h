//
// Created by gustavo on 24/06/2026.
//

#ifndef TRAB2_OC_NOC_SPIN_RSPIN_ROUTER_H
#define TRAB2_OC_NOC_SPIN_RSPIN_ROUTER_H
#include <systemc.h>
#include <queue>
#include "spin_flit.h"

enum Dir { DIR_NONE, DIR_UP, DIR_DOWN };
struct LockInfo { Dir dir; int port; };

SC_MODULE(RSPIN_Router) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    sc_in<SpinFlit>  up_in[4];
    sc_out<SpinFlit> up_out[4];
    sc_in<SpinFlit>  down_in[4];
    sc_out<SpinFlit> down_out[4];

    // Sinais de Crédito
    sc_in<bool>  up_credit_in[4];
    sc_out<bool> up_credit_out[4];
    sc_in<bool>  down_credit_in[4];
    sc_out<bool> down_credit_out[4];

    int my_id;
    int level;

    // Buffers e Máquina de Estados Wormhole
    std::queue<SpinFlit> in_buf_up[4];
    std::queue<SpinFlit> in_buf_down[4];

    int credits_up_out[4];
    int credits_down_out[4];

    bool out_up_busy[4];
    bool out_down_busy[4];

    // Tabela que indica qual a porta de saída que uma entrada "trancou" (0-3: Down, 4-7: Up)
    LockInfo input_locks[8];

    void routing_logic();

    SC_HAS_PROCESS(RSPIN_Router);
    RSPIN_Router(sc_module_name name, int id, int lvl);
};
#endif //TRAB2_OC_NOC_SPIN_RSPIN_ROUTER_H
