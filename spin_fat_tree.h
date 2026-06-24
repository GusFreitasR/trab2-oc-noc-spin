#include <systemc.h>
#include "rspin_router.h"
#include "terminal.h"

SC_MODULE(Spin_Fat_Tree) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    RSPIN_Router* leaf_routers[4];
    RSPIN_Router* root_routers[4];
    Terminal* terminals[16];

    // Fios de DADOS
    sc_signal<SpinFlit> link_up_tx[4][4];
    sc_signal<SpinFlit> link_down_tx[4][4];
    sc_signal<SpinFlit> link_term_tx[16];
    sc_signal<SpinFlit> link_term_rx[16];

    // Fios de CRÉDITOS
    sc_signal<bool> credit_up_tx[4][4];
    sc_signal<bool> credit_down_tx[4][4];
    sc_signal<bool> credit_term_tx[16];
    sc_signal<bool> credit_term_rx[16];

    sc_signal<SpinFlit> dummy_data_in[4][4];
    sc_signal<SpinFlit> dummy_data_out[4][4];
    sc_signal<bool>     dummy_credit_in[4][4];
    sc_signal<bool>     dummy_credit_out[4][4];

    SC_HAS_PROCESS(Spin_Fat_Tree);
    Spin_Fat_Tree(sc_module_name name);
    ~Spin_Fat_Tree();

    void configure_traffic(int src_id, int dest_id);
};