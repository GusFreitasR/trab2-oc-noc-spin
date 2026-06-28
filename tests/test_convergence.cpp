#include "test_harness.hpp"

int sc_main(int argc, char* argv[]) {
    return spintest::run_scenario(
        "Convergence of two flows on the same destination (T0,T8 -> T7)",
        {{0, 7}, {8, 7}});
}
