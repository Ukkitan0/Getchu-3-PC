cmake_minimum_required(VERSION 3.22)
project(Getchu3Native LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# NATIVE HOST-OS CODE ONLY. Absolutely no ps2-web-recomp, PCSX2,
# EE interpreter, IOP interpreter, GIF/VIF/GS/DMA hardware models.
# Embed official, sha256-pinned Lua 5.0.3 as HOST native C code.
# This is a scripting-language runtime, NOT any PlayStation emulation.
include(FetchContent)
FetchContent_Declare(Lua50
  URL "https://www.lua.org/ftp/lua-5.0.3.tar.gz"
  URL_HASH "SHA256=1193a61b0e08acaa6eee0eecf29709179ee49c71baebc59b682a25c3b5a45671"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(Lua50)
if(NOT lua50_POPULATED)
  FetchContent_Populate(Lua50)
endif()
set(LUA50_CORE
  lapi lcode ldebug ldo ldump lfunc lgc llex lmem lobject
  lopcodes lparser lstate lstring ltable ltm lundump lvm lzio
)
set(LUA50_LIBS lauxlib lbaselib lstrlib lmathlib ltablib)
set(LUA50_SRC)
foreach(file IN LISTS LUA50_CORE)
  list(APPEND LUA50_SRC "${lua50_SOURCE_DIR}/src/${file}.c")
endforeach()
foreach(file IN LISTS LUA50_LIBS)
  list(APPEND LUA50_SRC "${lua50_SOURCE_DIR}/src/lib/${file}.c")
endforeach()
add_library(Getchu3Lua50 STATIC ${LUA50_SRC})
target_include_directories(Getchu3Lua50 PUBLIC "${lua50_SOURCE_DIR}/include")
target_include_directories(Getchu3Lua50 PRIVATE "${lua50_SOURCE_DIR}/src")
set_target_properties(Getchu3Lua50 PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED ON)

add_library(Getchu3Assets STATIC src/archive.cpp src/tim2.cpp src/stage_scripts.cpp src/player_state.cpp src/lua_stage.cpp)
target_include_directories(Getchu3Assets PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(Getchu3Assets PUBLIC Getchu3Lua50)
if(MSVC)
    target_compile_options(Getchu3Assets PRIVATE /W4 /permissive-)
else()
    target_compile_options(Getchu3Assets PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(Getchu3AssetProbe src/asset_probe.cpp)
target_link_libraries(Getchu3AssetProbe PRIVATE Getchu3Assets)
if(WIN32)
    add_executable(Getchu3Native WIN32 src/app_win32.cpp)
    target_link_libraries(Getchu3Native PRIVATE Getchu3Assets d3d11 dxgi d3dcompiler
                          user32 gdi32 comdlg32)
    target_compile_definitions(Getchu3Native PRIVATE UNICODE _UNICODE)
    if(MSVC)
        target_compile_options(Getchu3Native PRIVATE /W4 /permissive-)
    endif()
endif()

enable_testing()
add_executable(Getchu3ArchiveTests tests/archive_test.cpp)
target_link_libraries(Getchu3ArchiveTests PRIVATE Getchu3Assets)
add_test(NAME NativeISOAndVFIArchiveRoundtrip COMMAND Getchu3ArchiveTests)
add_executable(Getchu3TextureTests tests/tim2_test.cpp)
target_link_libraries(Getchu3TextureTests PRIVATE Getchu3Assets)
add_test(NAME NativeTIM2TextureDecoding COMMAND Getchu3TextureTests)
add_executable(Getchu3StageProbe src/stage_probe.cpp)
target_link_libraries(Getchu3StageProbe PRIVATE Getchu3Assets)
add_executable(Getchu3StageScriptTests tests/stage_scripts_test.cpp)
target_link_libraries(Getchu3StageScriptTests PRIVATE Getchu3Assets)
add_test(NAME NativeLua50StageScriptDiscovery COMMAND Getchu3StageScriptTests)
add_executable(Getchu3PlayerStateTests tests/player_state_test.cpp)
target_link_libraries(Getchu3PlayerStateTests PRIVATE Getchu3Assets)
add_test(NAME NativeRecoveredPlayerHealthAndSaveLayout COMMAND Getchu3PlayerStateTests)

add_executable(Getchu3LuaStageRunner src/lua_stage_runner.cpp)
target_link_libraries(Getchu3LuaStageRunner PRIVATE Getchu3Assets)
add_executable(Getchu3LuaRuntimeTests tests/lua_runtime_test.cpp)
target_link_libraries(Getchu3LuaRuntimeTests PRIVATE Getchu3Assets)
add_test(NAME NativeLua50RealStageExecution COMMAND Getchu3LuaRuntimeTests)
