#include "test_harness.hpp"

int sc_main(int argc, char* argv[]) {
    return spintest::run_scenario(
        "Four flows serialized to a single destination (T0..T3 -> T10)",
        {{0, 10}, {1, 10}, {2, 10}, {3, 10}});
}
