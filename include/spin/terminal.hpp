#pragma once

#include <systemc.h>

#include "spin/config.hpp"
#include "spin/flit.hpp"

namespace spin {

SC_MODULE(Terminal) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    sc_out<Flit> tx;
    sc_in<Flit> rx;

    sc_in<bool> credit_in;
    sc_out<bool> credit_out;

    Terminal(sc_module_name name, int id);

    void set_target(int dest_id);
    int packets_received() const { return packets_received_; }

 private:
    void process();

    int id_;
    int target_id_{-1};
    int flits_to_send_{0};
    int credits_{kBufferDepth};
    int packets_received_{0};

    SC_HAS_PROCESS(Terminal);
};

}
