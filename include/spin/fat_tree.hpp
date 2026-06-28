#pragma once

#include <array>
#include <memory>

#include <systemc.h>

#include "spin/config.hpp"
#include "spin/flit.hpp"
#include "spin/router.hpp"
#include "spin/terminal.hpp"

namespace spin {

SC_MODULE(FatTree) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    explicit FatTree(sc_module_name name);

    void configure_traffic(int src_id, int dest_id);
    int packets_received(int terminal_id) const;

 private:
    std::array<std::unique_ptr<Router>, kNumLeaves> leaves_;
    std::array<std::unique_ptr<Router>, kNumRoots> roots_;
    std::array<std::unique_ptr<Terminal>, kNumTerminals> terminals_;

    sc_signal<Flit> link_up_[kNumLeaves][kNumRoots];
    sc_signal<Flit> link_down_[kNumRoots][kNumLeaves];
    sc_signal<Flit> term_tx_[kNumTerminals];
    sc_signal<Flit> term_rx_[kNumTerminals];

    sc_signal<bool> credit_up_[kNumLeaves][kNumRoots];
    sc_signal<bool> credit_down_[kNumRoots][kNumLeaves];
    sc_signal<bool> credit_term_tx_[kNumTerminals];
    sc_signal<bool> credit_term_rx_[kNumTerminals];

    sc_signal<Flit> stub_in_[kNumRoots][kArity];
    sc_signal<Flit> stub_out_[kNumRoots][kArity];
    sc_signal<bool> stub_credit_in_[kNumRoots][kArity];
    sc_signal<bool> stub_credit_out_[kNumRoots][kArity];
};

}
