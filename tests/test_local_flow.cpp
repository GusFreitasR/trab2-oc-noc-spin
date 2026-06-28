#include "test_harness.hpp"

int sc_main(int argc, char* argv[]) {
    return spintest::run_scenario(
        "Local flow within the same leaf, no upward routing (T0 -> T3)",
        {{0, 3}});
}
