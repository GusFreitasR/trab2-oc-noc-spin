#include "spin_fat_tree.h"

Spin_Fat_Tree::Spin_Fat_Tree(sc_module_name name) : sc_module(name) {
    char buf[30];

    for (int i = 0; i < 4; i++) {
        sprintf(buf, "Leaf_Router_%d", i); leaf_routers[i] = new RSPIN_Router(buf, i, 1);
        leaf_routers[i]->clk(clk); leaf_routers[i]->rst(rst);

        sprintf(buf, "Root_Router_%d", i); root_routers[i] = new RSPIN_Router(buf, i, 2);
        root_routers[i]->clk(clk); root_routers[i]->rst(rst);
    }
    for (int i = 0; i < 16; i++) {
        sprintf(buf, "Terminal_%d", i); terminals[i] = new Terminal(buf, i);
        terminals[i]->clk(clk); terminals[i]->rst(rst);
    }

    // CABEAMENTO DADOS E CRÉDITOS
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // Sinais Folha -> Raiz
            leaf_routers[i]->up_out[j](link_up_tx[i][j]);
            root_routers[j]->down_in[i](link_up_tx[i][j]);
            // O crédito da Raiz -> Folha
            root_routers[j]->down_credit_out[i](credit_up_tx[i][j]);
            leaf_routers[i]->up_credit_in[j](credit_up_tx[i][j]);

            // Sinais Raiz -> Folha
            root_routers[j]->down_out[i](link_down_tx[j][i]);
            leaf_routers[i]->up_in[j](link_down_tx[j][i]);
            // O crédito da Folha -> Raiz
            leaf_routers[i]->up_credit_out[j](credit_down_tx[j][i]);
            root_routers[j]->down_credit_in[i](credit_down_tx[j][i]);

            // trava
            root_routers[i]->up_in[j](dummy_data_in[i][j]);
            root_routers[i]->up_out[j](dummy_data_out[i][j]);
            root_routers[i]->up_credit_in[j](dummy_credit_in[i][j]);
            root_routers[i]->up_credit_out[j](dummy_credit_out[i][j]);
        }
    }

    for (int i = 0; i < 16; i++) {
        int leaf_id = i / 4; int port_id = i % 4;

        terminals[i]->tx(link_term_tx[i]);
        leaf_routers[leaf_id]->down_in[port_id](link_term_tx[i]);

        leaf_routers[leaf_id]->down_credit_out[port_id](credit_term_tx[i]);
        terminals[i]->credit_in(credit_term_tx[i]);

        leaf_routers[leaf_id]->down_out[port_id](link_term_rx[i]);
        terminals[i]->rx(link_term_rx[i]);

        terminals[i]->credit_out(credit_term_rx[i]);
        leaf_routers[leaf_id]->down_credit_in[port_id](credit_term_rx[i]);
    }
}

void Spin_Fat_Tree::configure_traffic(int src_id, int dest_id) {
    if (src_id >= 0 && src_id < 16) terminals[src_id]->set_target(dest_id);
}

Spin_Fat_Tree::~Spin_Fat_Tree() {
    for (int i = 0; i < 4; i++) { delete leaf_routers[i]; delete root_routers[i]; }
    for (int i = 0; i < 16; i++) delete terminals[i];
}