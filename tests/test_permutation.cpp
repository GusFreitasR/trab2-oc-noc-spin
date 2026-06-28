#include "test_harness.hpp"

int sc_main(int argc, char* argv[]) {
    return spintest::run_scenario(
        "Concurrent load: 8 simultaneous disjoint flows",
        {{0, 4}, {1, 5}, {2, 6}, {3, 7},
         {8, 12}, {9, 13}, {10, 14}, {11, 15}});
}
