#pragma once

#include <array>
#include <queue>
#include <random>
#include <string>

#include <systemc.h>

#include "spin/config.hpp"
#include "spin/flit.hpp"

namespace spin {

enum class Direction : std::uint8_t { kNone, kUp, kDown };

SC_MODULE(Router) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    sc_in<Flit> up_in[kArity];
    sc_out<Flit> up_out[kArity];
    sc_in<Flit> down_in[kArity];
    sc_out<Flit> down_out[kArity];

    sc_in<bool> up_credit_in[kArity];
    sc_out<bool> up_credit_out[kArity];
    sc_in<bool> down_credit_in[kArity];
    sc_out<bool> down_credit_out[kArity];

    Router(sc_module_name name, int id, int level);

 private:
    struct Lock {
        Direction dir{Direction::kNone};
        int port{-1};
    };

    void process();
    void reset_state();
    const std::array<int, kNumPorts>& draw_arbitration_order();

    int id_;
    int level_;

    std::array<std::queue<Flit>, kArity> in_up_;
    std::array<std::queue<Flit>, kArity> in_down_;
    std::array<int, kArity> credits_up_{};
    std::array<int, kArity> credits_down_{};
    std::array<bool, kArity> busy_up_{};
    std::array<bool, kArity> busy_down_{};
    std::array<Lock, kNumPorts> locks_{};

    std::array<int, kNumPorts> order_{};
    std::string roulette_label_;
    std::mt19937 rng_;

    SC_HAS_PROCESS(Router);
};

}
