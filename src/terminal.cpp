#include "spin/terminal.hpp"

#include "spin/trace.hpp"
#include <string>

namespace spin {

Terminal::Terminal(sc_module_name name, int id) : sc_module(name), id_(id) {
    SC_METHOD(process);
    sensitive << clk.pos();
}

void Terminal::set_target(int dest_id) {
    target_id_ = dest_id;
    flits_to_send_ = kFlitsPerMessage;
}

void Terminal::process() {
    if (rst.read()) {
        tx.write(Flit{});
        credit_out.write(false);
        credits_ = kBufferDepth;
        return;
    }

    if (credit_in.read()) {
        ++credits_;
    }

    if (flits_to_send_ > 0 && credits_ > 0) {
        Flit flit;
        flit.valid = true;
        flit.src = id_;
        flit.dest = target_id_;

        if (flits_to_send_ == kFlitsPerMessage) {
            flit.type = FlitType::kHeader;
            flit.data = ">> MESSAGE_START <<";
            spin::trace() << "[ PACKET STARTED ] @ " << sc_time_stamp()
                      << " | Source: " << id_ << " Destination: " << target_id_ << '\n';
        } else if (flits_to_send_ == 1) {
            flit.type = FlitType::kTail;
            flit.data = ">> MESSAGE_END <<";
        } else {
            flit.type = FlitType::kPayload;
            const int index = (flits_to_send_ == kFlitsPerMessage - 1) ? 1 : 2;
            flit.data = "Hello world :) #" + std::to_string(index);
        }

        tx.write(flit);
        --credits_;
        --flits_to_send_;
        return;
    }

    tx.write(Flit{});

    const Flit incoming = rx.read();
    bool consumed = false;
    if (incoming.valid) {
        consumed = true;
        spin::trace() << "    <<< @ " << sc_time_stamp() << " | Terminal " << id_
                  << " RECEIVED [" << to_string(incoming.type) << "] | Data: "
                  << incoming.data << '\n';
        if (incoming.type == FlitType::kTail) {
            ++packets_received_;
            spin::trace() << "        -> (Packet fully received and reassembled!)\n";
        }
    }
    credit_out.write(consumed);
}

}
