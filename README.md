# Network-on-Chip (NoC) SPIN Simulator in SystemC

Cycle-accurate SystemC simulator that models the SPIN Network-on-Chip (NoC)
architecture over a quaternary Fat-Tree topology. It implements credit-based flow
control, wormhole switching and hybrid probabilistic arbitration with adaptive
routing.

---

## Architecture

* **Quaternary Fat-Tree topology:** 2 levels, 8 routers (4 leaf + 4 root)
  interconnecting 16 terminals.
* **Wormhole switching:** messages split into *flits* (`Header`, `Payload`,
  `Tail`); the *header* allocates the route and locks the channel.
* **Credit-based flow control:** a flit is sent only if the neighboring input
  FIFO buffer (depth 4) has guaranteed space.
* **Hybrid probabilistic arbiter:** a per-cycle draw (`std::mt19937`) reorders
  the port priorities (50% UP, 30% DOWN, 15% mixed-UP, 5% mixed-DOWN),
  mitigating starvation.
* **Two-phase routing:** adaptive upward phase (link with the most free credits)
  and deterministic downward phase (via `dest / 4` and `dest % 4`).

The model guarantees **lossless delivery for N concurrent flows**, including when
multiple flows converge on the same output port (a single write per port per
cycle prevents channel overwriting).

---

## Requirements

* Linux, GCC/G++ with C++17, CMake (>= 3.16).
* **Accellera SystemC 3.0.2.** `CMakeLists.txt` searches for SystemC in this
  order: the `SYSTEMC_HOME` variable, `~/.local/systemc-3.0.2`,
  `/usr/local/systemc-3.0.2`. To point it manually:
  `cmake -DSYSTEMC_HOME=/path/to/systemc ..`

---

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## Running and Testing

The simulation scenarios live in `tests/`. Each test is a full *testbench*
(`sc_main`) that injects a scenario, **automatically verifies** the delivery of
every packet (returns a non-zero exit code on failure) and prints a clean,
aligned report with only what is essential for evaluation.

Run all tests with automatic verification (CTest):

```bash
cd build && ctest -V                              
```

| Test                 | Scenario          | Verifies                          |
|----------------------|-------------------|-----------------------------------|
| `test_single_flow`   | `T1 -> T14`       | delivery crossing the root        |
| `test_local_flow`    | `T0 -> T3`        | local routing within the same leaf|
| `test_convergence`   | `T0,T8 -> T7`     | no loss under output contention   |
| `test_permutation`   | 8 disjoint flows  | concurrent load without deadlock  |
| `test_many_to_one`   | `T0..T3 -> T10`   | serialization to one destination  |