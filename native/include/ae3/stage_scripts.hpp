#pragma once
#include "ae3/archive.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ae3 {
enum class ScriptKind {
    lua50_bytecode,
    other_lua_bytecode,
    lua_source,
    unsupported
};
struct ScriptDescriptor {
    std::string path;
    ScriptKind kind = ScriptKind::unsupported;
    unsigned bytecodeVersion = 0;
    std::uint64_t bytes = 0;
    bool areaScript = false;
};
ScriptDescriptor inspectScript(const std::string& name,
                               const std::vector<std::uint8_t>& contents);
std::vector<ScriptDescriptor> discoverArchiveScripts(Archive& game,
                                                      std::size_t maxScriptBytes=8*1024*1024);
std::vector<ScriptDescriptor> discoverLooseScripts(const std::filesystem::path& folder,
                                                    std::size_t maxScriptBytes=8*1024*1024);
const char* scriptKindName(ScriptKind kind);
}
