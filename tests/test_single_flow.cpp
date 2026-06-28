#include "test_harness.hpp"

int sc_main(int argc, char* argv[]) {
    return spintest::run_scenario(
        "Single flow crossing the root (T1 -> T14)",
        {{1, 14}});
}
