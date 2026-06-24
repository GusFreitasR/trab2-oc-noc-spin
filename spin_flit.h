#ifndef SPIN_FLIT_H
#define SPIN_FLIT_H

#include <systemc.h>
#include <iostream>

enum FlitType { FLIT_NONE, FLIT_HEADER, FLIT_PAYLOAD, FLIT_TAIL };

struct SpinFlit {
    FlitType type;
    sc_uint<10> dest; 
    sc_uint<10> src;
    std::string data;
    bool valid;

    SpinFlit() : type(FLIT_NONE), dest(0), src(0), data(""), valid(false) {}

    inline bool operator == (const SpinFlit& rhs) const {
        return (rhs.type == type && rhs.valid == valid && rhs.dest == dest && rhs.src == src);
    }
};

inline void sc_trace(sc_trace_file* f, const SpinFlit& flit, const std::string& name) {
    sc_trace(f, flit.valid, name + ".valid");
}

inline std::ostream& operator << (std::ostream& os, const SpinFlit& f) {
    std::string tipo = (f.type == FLIT_HEADER) ? "HEADER" : (f.type == FLIT_TAIL) ? "TAIL" : "PAYLOAD";
    os << "[" << tipo << " | Val: " << f.valid << "]";
    return os;
}

#endif // SPIN_FLIT_H