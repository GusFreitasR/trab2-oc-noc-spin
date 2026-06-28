#include "spin/trace.hpp"

#include <cstdlib>
#include <iostream>
#include <streambuf>
#include <string>

namespace spin {

namespace {

class NullBuffer : public std::streambuf {
 public:
    int overflow(int c) override { return c; }
};

NullBuffer g_null_buffer;
std::ostream g_null_stream(&g_null_buffer);

bool read_env_flag() {
    const char* value = std::getenv("SPIN_TRACE");
    return value != nullptr && std::string(value) != "0";
}

const bool g_enabled = read_env_flag();

}

std::ostream& trace() { return g_enabled ? std::cout : g_null_stream; }

}
