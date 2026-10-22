#include "Modules/ModuleManager.h"

// The packaged Windows game can load the source-backed original Lua 5.0 DLL;
// the editor-only ISO importer is deliberately not a runtime dependency.
IMPLEMENT_MODULE(FDefaultModuleImpl, Getchu3OriginalLua)
