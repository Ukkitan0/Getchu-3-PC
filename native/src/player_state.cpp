#include "ae3/player_state.hpp"
#include <algorithm>
namespace ae3 {
float changePlayerHealth(PlayerProgress& state, float delta) {
    // Original ChangeHP(delta) behavior reconstructed from USA retail
    // executable; use native C++ data rather than guest MIPS execution.
    state.player_life = std::max(0.0f, std::min(100.0f, state.player_life + delta));
    return state.player_life;
}
}
