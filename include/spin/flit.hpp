#pragma once

#include <cstdint>
#include <ostream>
#include <string>

#include <systemc.h>

namespace spin {

enum class FlitType : std::uint8_t { kNone, kHeader, kPayload, kTail };

struct Flit {
    FlitType type{FlitType::kNone};
    sc_uint<10> dest{0};
    sc_uint<10> src{0};
    std::string data{};
    bool valid{false};

    bool operator==(const Flit& rhs) const {
        return type == rhs.type && valid == rhs.valid && dest == rhs.dest &&
               src == rhs.src && data == rhs.data;
    }
};

inline const char* to_string(FlitType type) {
    switch (type) {
        case FlitType::kHeader:  return "HEADER";
        case FlitType::kPayload: return "PAYLOAD";
        case FlitType::kTail:    return "TAIL";
        default:                 return "NONE";
    }
}

inline void sc_trace(sc_trace_file* tf, const Flit& flit, const std::string& name) {
    sc_trace(tf, flit.valid, name + ".valid");
}

inline std::ostream& operator<<(std::ostream& os, const Flit& flit) {
    return os << '[' << to_string(flit.type) << " | Val: " << flit.valid << ']';
}

}
