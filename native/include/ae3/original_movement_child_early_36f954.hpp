#pragma once
#include "ae3/original_movement_child_30c57c.hpp"
namespace ae3 {
// Source 0x36F954 early-return gates, called by 0x557EC8 with
// a0=child+0x10 and source f12=2.0f from original 0x704B04.
// TRUE: retail source definitely returns 1, and 0x557E30
// returns status 2. FALSE: later original branches are required
// and MUST NOT be mistaken for original return zero.
bool originalMovementChildTerminal36F954(
    const OriginalMovementChildD0_30c57c& originalChild);
} // namespace ae3
