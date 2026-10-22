#include "windows_runtime_boot_status.hpp"
#ifdef NDEBUG
#undef NDEBUG // CTest Release builds must still exercise assertions.
#endif
#include <cassert>
#include <string>
int main() {
    using namespace ae3::runtime;
    BootJournal log;
    assert(!log.blocked());
    assert(!log.originalEngineLoopReached());
    assert(log.record(BootCheckpoint::windows_window,BootResult::passed,"Win32 ready"));
    assert(log.record(BootCheckpoint::original_disc,BootResult::passed,"Retail archive read"));
    assert(log.record(BootCheckpoint::original_lua_setup,BootResult::passed,"Original setupArea"));
    assert(!log.originalEngineLoopReached());
    assert(log.record(BootCheckpoint::original_engine_loop,BootResult::blocked,
                      "Retail engine frame dispatcher missing|not playable\n"));
    assert(log.blocked());
    assert(!log.record(BootCheckpoint::original_actors,BootResult::passed,"false success"));
    assert(log.observations().size()==4);
    assert(log.lastBlocker().find("dispatcher")!=std::string::npos);
    const std::string trace=log.trace();
    assert(trace.find("RETAIL_ENGINE_LOOP|BLOCKED|")!=std::string::npos);
    assert(trace.find("missing not playable \n")!=std::string::npos);
    log.reset();
    assert(!log.blocked());
    assert(log.observations().empty());
    assert(!log.originalEngineLoopReached());
}
