#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <systemc.h>

#include "spin/config.hpp"
#include "spin/fat_tree.hpp"

namespace spintest {

struct Flow {
    int src;
    int dest;
};

inline int run_scenario(const std::string& title,
                        const std::vector<Flow>& flows,
                        int simulation_ns = 1000) {
    sc_clock clk("clk", spin::kClockPeriodNs, SC_NS);
    sc_signal<bool> rst;
    spin::FatTree noc("noc");
    noc.clk(clk);
    noc.rst(rst);

    sc_start(0, SC_NS);
    rst.write(true);
    sc_start(20, SC_NS);
    rst.write(false);
    sc_start(10, SC_NS);

    std::map<int, int> expected;
    for (const Flow& flow : flows) {
        noc.configure_traffic(flow.src, flow.dest);
        ++expected[flow.dest];
    }

    sc_start(simulation_ns, SC_NS);

    const std::string heavy(56, '=');
    const std::string light(56, '-');

    std::cout << '\n' << heavy << '\n';
    std::cout << "  Scenario: " << title << '\n';
    std::cout << light << '\n';
    std::cout << "  Injected flows (" << flows.size() << "):\n";
    for (const Flow& flow : flows) {
        std::cout << "      T" << std::setw(2) << flow.src
                  << "  ->  T" << std::setw(2) << flow.dest << '\n';
    }
    std::cout << light << '\n';
    std::cout << "  Delivery per destination:\n";

    int delivered = 0;
    int expected_total = 0;
    int failures = 0;
    for (const auto& [dest, count] : expected) {
        const int received = noc.packets_received(dest);
        const bool ok = (received == count);
        delivered += received;
        expected_total += count;
        if (!ok) ++failures;
        std::cout << "      Terminal " << std::setw(2) << dest << "  :  "
                  << received << " / " << count << "   "
                  << (ok ? "OK" : "FAIL") << '\n';
    }

    std::cout << light << '\n';
    std::cout << "  RESULT: " << (failures == 0 ? "PASSED" : "FAILED")
              << "   (" << delivered << " / " << expected_total
              << " packets delivered)\n";
    std::cout << heavy << "\n\n";

    return failures == 0 ? 0 : 1;
}

}
