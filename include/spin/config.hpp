#pragma once

#include <cstdint>

namespace spin {

inline constexpr int kArity = 4;
inline constexpr int kBufferDepth = 4;
inline constexpr int kNumLeaves = 4;
inline constexpr int kNumRoots = 4;
inline constexpr int kTerminalsPerLeaf = 4;
inline constexpr int kNumTerminals = kNumLeaves * kTerminalsPerLeaf;
inline constexpr int kFlitsPerMessage = 4;
inline constexpr int kClockPeriodNs = 10;
inline constexpr int kNumPorts = 2 * kArity;

}
