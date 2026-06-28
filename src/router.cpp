#include "spin/router.hpp"

#include "spin/trace.hpp"

namespace spin {

Router::Router(sc_module_name name, int id, int level)
    : sc_module(name),
      id_(id),
      level_(level),
      rng_(static_cast<std::mt19937::result_type>(level * 100 + id + 1)) {
    credits_up_.fill(kBufferDepth);
    credits_down_.fill(kBufferDepth);
    SC_METHOD(process);
    sensitive << clk.pos() << rst;
}

void Router::reset_state() {
    for (int i = 0; i < kArity; ++i) {
        up_out[i].write(Flit{});
        down_out[i].write(Flit{});
        up_credit_out[i].write(false);
        down_credit_out[i].write(false);
        credits_up_[i] = kBufferDepth;
        credits_down_[i] = kBufferDepth;
        busy_up_[i] = false;
        busy_down_[i] = false;
        while (!in_up_[i].empty()) in_up_[i].pop();
        while (!in_down_[i].empty()) in_down_[i].pop();
    }
    for (auto& lock : locks_) {
        lock = Lock{};
    }
}

const std::array<int, kNumPorts>& Router::draw_arbitration_order() {
    const int spin = std::uniform_int_distribution<int>(0, 99)(rng_);
    if (spin < 50) {
        order_ = {4, 5, 6, 7, 0, 1, 2, 3};
        roulette_label_ = "rolled 50% (UP priority)";
    } else if (spin < 80) {
        order_ = {0, 1, 2, 3, 4, 5, 6, 7};
        roulette_label_ = "rolled 30% (DOWN priority)";
    } else if (spin < 95) {
        order_ = {4, 0, 5, 1, 6, 2, 7, 3};
        roulette_label_ = "rolled 15% (Mixed UP)";
    } else {
        order_ = {0, 4, 1, 5, 2, 6, 3, 7};
        roulette_label_ = "rolled 5% (Mixed DOWN)";
    }
    return order_;
}

void Router::process() {
    if (rst.read()) {
        reset_state();
        return;
    }

    for (int i = 0; i < kArity; ++i) {
        if (up_credit_in[i].read()) ++credits_up_[i];
        if (down_credit_in[i].read()) ++credits_down_[i];
    }

    for (int i = 0; i < kArity; ++i) {
        const Flit from_up = up_in[i].read();
        if (from_up.valid) in_up_[i].push(from_up);
        const Flit from_down = down_in[i].read();
        if (from_down.valid) in_down_[i].push(from_down);
    }

    std::array<Flit, kArity> out_up{};
    std::array<Flit, kArity> out_down{};
    std::array<bool, kArity> credit_back_up{};
    std::array<bool, kArity> credit_back_down{};
    std::array<bool, kArity> written_up{};
    std::array<bool, kArity> written_down{};

    const std::array<int, kNumPorts>& order = draw_arbitration_order();

    for (int slot = 0; slot < kNumPorts; ++slot) {
        const int buffer = order[slot];
        const bool is_down_input = buffer < kArity;
        const int local = is_down_input ? buffer : buffer - kArity;
        std::queue<Flit>& queue = is_down_input ? in_down_[local] : in_up_[local];

        if (queue.empty()) continue;

        const Flit flit = queue.front();

        if (flit.type == FlitType::kHeader && locks_[buffer].dir == Direction::kNone) {
            const int leaf_dest = static_cast<int>(flit.dest) / kTerminalsPerLeaf;
            const int term_dest = static_cast<int>(flit.dest) % kTerminalsPerLeaf;
            const bool must_go_up = (level_ == 1 && leaf_dest != id_);

            spin::trace() << "  [ARBITER] Router Level " << level_ << " ID " << id_
                      << " | " << roulette_label_ << " -> Serving Input "
                      << buffer << '\n';

            if (must_go_up) {
                int best_port = -1;
                int max_credits = -1;
                for (int p = 0; p < kArity; ++p) {
                    if (!busy_up_[p] && credits_up_[p] > max_credits) {
                        max_credits = credits_up_[p];
                        best_port = p;
                    }
                }
                if (best_port != -1) {
                    locks_[buffer] = Lock{Direction::kUp, best_port};
                    busy_up_[best_port] = true;
                    spin::trace() << "      -> [HW] Adaptive routing locked UP port "
                              << best_port << '\n';
                }
            } else {
                const int target = (level_ == 1) ? term_dest : leaf_dest;
                if (!busy_down_[target]) {
                    locks_[buffer] = Lock{Direction::kDown, target};
                    busy_down_[target] = true;
                }
            }
        }

        if (locks_[buffer].dir == Direction::kNone) continue;

        const Direction dir = locks_[buffer].dir;
        const int port = locks_[buffer].port;
        const bool up = (dir == Direction::kUp);

        const bool has_credit = up ? credits_up_[port] > 0 : credits_down_[port] > 0;
        const bool slot_free = up ? !written_up[port] : !written_down[port];
        if (!has_credit || !slot_free) continue;

        if (up) {
            out_up[port] = flit;
            written_up[port] = true;
            --credits_up_[port];
        } else {
            out_down[port] = flit;
            written_down[port] = true;
            --credits_down_[port];
        }
        queue.pop();

        if (is_down_input) {
            credit_back_down[local] = true;
        } else {
            credit_back_up[local] = true;
        }

        spin::trace() << "    >> @ " << sc_time_stamp() << " | Router Level " << level_
                  << " (ID " << id_ << ") dispatched [" << to_string(flit.type)
                  << "] -> Port " << (up ? "UP " : "DOWN ") << port << '\n';

        if (flit.type == FlitType::kTail) {
            locks_[buffer] = Lock{};
            if (up) {
                busy_up_[port] = false;
            } else {
                busy_down_[port] = false;
            }
        }
    }

    for (int i = 0; i < kArity; ++i) {
        up_out[i].write(out_up[i]);
        down_out[i].write(out_down[i]);
        up_credit_out[i].write(credit_back_up[i]);
        down_credit_out[i].write(credit_back_down[i]);
    }
}

}
