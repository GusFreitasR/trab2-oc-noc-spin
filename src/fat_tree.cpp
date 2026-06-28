#include "spin/fat_tree.hpp"

#include <string>

namespace spin {

FatTree::FatTree(sc_module_name name) : sc_module(name) {
    for (int i = 0; i < kNumLeaves; ++i) {
        leaves_[i] = std::make_unique<Router>(
            ("Leaf_Router_" + std::to_string(i)).c_str(), i, 1);
        leaves_[i]->clk(clk);
        leaves_[i]->rst(rst);
    }
    for (int i = 0; i < kNumRoots; ++i) {
        roots_[i] = std::make_unique<Router>(
            ("Root_Router_" + std::to_string(i)).c_str(), i, 2);
        roots_[i]->clk(clk);
        roots_[i]->rst(rst);
    }
    for (int i = 0; i < kNumTerminals; ++i) {
        terminals_[i] = std::make_unique<Terminal>(
            ("Terminal_" + std::to_string(i)).c_str(), i);
        terminals_[i]->clk(clk);
        terminals_[i]->rst(rst);
    }

    for (int leaf = 0; leaf < kNumLeaves; ++leaf) {
        for (int root = 0; root < kNumRoots; ++root) {
            leaves_[leaf]->up_out[root](link_up_[leaf][root]);
            roots_[root]->down_in[leaf](link_up_[leaf][root]);
            roots_[root]->down_credit_out[leaf](credit_up_[leaf][root]);
            leaves_[leaf]->up_credit_in[root](credit_up_[leaf][root]);

            roots_[root]->down_out[leaf](link_down_[root][leaf]);
            leaves_[leaf]->up_in[root](link_down_[root][leaf]);
            leaves_[leaf]->up_credit_out[root](credit_down_[root][leaf]);
            roots_[root]->down_credit_in[leaf](credit_down_[root][leaf]);
        }
    }

    for (int root = 0; root < kNumRoots; ++root) {
        for (int port = 0; port < kArity; ++port) {
            roots_[root]->up_in[port](stub_in_[root][port]);
            roots_[root]->up_out[port](stub_out_[root][port]);
            roots_[root]->up_credit_in[port](stub_credit_in_[root][port]);
            roots_[root]->up_credit_out[port](stub_credit_out_[root][port]);
        }
    }

    for (int i = 0; i < kNumTerminals; ++i) {
        const int leaf = i / kTerminalsPerLeaf;
        const int port = i % kTerminalsPerLeaf;

        terminals_[i]->tx(term_tx_[i]);
        leaves_[leaf]->down_in[port](term_tx_[i]);

        leaves_[leaf]->down_credit_out[port](credit_term_tx_[i]);
        terminals_[i]->credit_in(credit_term_tx_[i]);

        leaves_[leaf]->down_out[port](term_rx_[i]);
        terminals_[i]->rx(term_rx_[i]);

        terminals_[i]->credit_out(credit_term_rx_[i]);
        leaves_[leaf]->down_credit_in[port](credit_term_rx_[i]);
    }
}

void FatTree::configure_traffic(int src_id, int dest_id) {
    if (src_id >= 0 && src_id < kNumTerminals) {
        terminals_[src_id]->set_target(dest_id);
    }
}

int FatTree::packets_received(int terminal_id) const {
    if (terminal_id < 0 || terminal_id >= kNumTerminals) return 0;
    return terminals_[terminal_id]->packets_received();
}

}
