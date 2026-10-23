cmake_minimum_required(VERSION 3.22)
project(Getchu3OriginalLuaBridge LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Native original Lua 5.0 bridge ONLY. This does not configure unrelated
# glTF, zlib, graphics, input, emulator, or original-game runtime targets.
# Keep the official Lua source pin consistent with native/CMakeLists.txt.
include(FetchContent)
FetchContent_Declare(Getchu3Lua503
  URL "https://www.lua.org/ftp/lua-5.0.3.tar.gz"
  URL_HASH "SHA256=1193a61b0e08acaa6eee0eecf29709179ee49c71baebc59b682a25c3b5a45671"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Lua 5.0.3 has no CMakeLists.txt. Modern FetchContent_MakeAvailable
# correctly populates sources WITHOUT add_subdirectory for such archives.
# Verified locally with CMake 3.31 and FETCHCONTENT_SOURCE_DIR override.
# Avoid deprecated FetchContent_Populate under CMP0169.
FetchContent_MakeAvailable(Getchu3Lua503)
if(NOT EXISTS "${getchu3lua503_SOURCE_DIR}/src/lapi.c" OR
   NOT EXISTS "${getchu3lua503_SOURCE_DIR}/src/lib/lauxlib.c" OR
   NOT EXISTS "${getchu3lua503_SOURCE_DIR}/include/lua.h")
  message(FATAL_ERROR "Official Lua 5.0.3 archive missing expected source layout")
endif()
set(LUA503_CORE
  lapi lcode ldebug ldo ldump lfunc lgc llex lmem lobject
  lopcodes lparser lstate lstring ltable ltm lundump lvm lzio
)
set(LUA503_LIBS lauxlib lbaselib lstrlib lmathlib ltablib)
set(LUA503_SOURCES)
foreach(item IN LISTS LUA503_CORE)
  list(APPEND LUA503_SOURCES "${getchu3lua503_SOURCE_DIR}/src/${item}.c")
endforeach()
foreach(item IN LISTS LUA503_LIBS)
  list(APPEND LUA503_SOURCES "${getchu3lua503_SOURCE_DIR}/src/lib/${item}.c")
endforeach()

add_library(Getchu3Lua50 STATIC ${LUA503_SOURCES})
target_include_directories(Getchu3Lua50
  PUBLIC "${getchu3lua503_SOURCE_DIR}/include"
  PRIVATE "${getchu3lua503_SOURCE_DIR}/src")
set_target_properties(Getchu3Lua50 PROPERTIES
  C_STANDARD 90 C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON)

set(ORIGINAL_NATIVE_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(ORIGINAL_NATIVE_SOURCE "${ORIGINAL_NATIVE_ROOT}/src")
set(ORIGINAL_NATIVE_INCLUDE "${ORIGINAL_NATIVE_ROOT}/include")
set(ORIGINAL_LUA_STAGE_SOURCES
  "${ORIGINAL_NATIVE_SOURCE}/lua_stage.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/retail_lua50.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/player_state.cpp")

# Compile the ACTUAL recovered original Player virtual resource router and
# child controller lifecycle alongside Lua. These are genuine native C++
# source recoveries, not generic Unreal Actor replacements.
set(ORIGINAL_ACTOR_MANAGER_SOURCES
  "${ORIGINAL_NATIVE_SOURCE}/original_player_factory.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_child.cpp"
  # Original actor navigation initializer 0x57BCB8, source route-edge
  # lookup, type-2 target, exact source RNG / VU0 and 0x70 polygon views.
  # These are real recovered original functions, not actor substitutes.
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_initializer.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_route_follower.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_nav_route.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_type2_target.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_random_triangle.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_ray_geometry.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_game_random.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_spatial_index.cpp")
add_library(Getchu3OriginalLuaBridge SHARED
  "${ORIGINAL_NATIVE_SOURCE}/ae3_unreal_lua_bridge.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/ae3_original_manager_init_bridge.cpp"
  ${ORIGINAL_LUA_STAGE_SOURCES}
  ${ORIGINAL_ACTOR_MANAGER_SOURCES})
target_include_directories(Getchu3OriginalLuaBridge
  PUBLIC "${ORIGINAL_NATIVE_INCLUDE}")
target_link_libraries(Getchu3OriginalLuaBridge PRIVATE Getchu3Lua50)
target_compile_definitions(Getchu3OriginalLuaBridge PRIVATE AE3_LUA_BRIDGE_BUILD)
set_target_properties(Getchu3OriginalLuaBridge PROPERTIES
  OUTPUT_NAME "Getchu3OriginalLuaBridge"
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES)
if(UNIX AND NOT APPLE)
  target_link_libraries(Getchu3OriginalLuaBridge PRIVATE m)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(Getchu3OriginalUnrealLuaBridgeTests
    "${ORIGINAL_NATIVE_ROOT}/tests/ae3_unreal_lua_bridge_test.cpp")
  target_include_directories(Getchu3OriginalUnrealLuaBridgeTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalUnrealLuaBridgeTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalLua50UnrealHostBridgeFailClosed
    COMMAND Getchu3OriginalUnrealLuaBridgeTests)

  add_executable(Getchu3OriginalPlayerResourceTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_player_factory_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_player_factory.cpp")
  target_include_directories(Getchu3OriginalPlayerResourceTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalPlayerResourceVirtual2
    COMMAND Getchu3OriginalPlayerResourceTests)

  add_executable(Getchu3OriginalManagerChildTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_child_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_child.cpp")
  target_include_directories(Getchu3OriginalManagerChildTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalManagerChildCtorLifecycle
    COMMAND Getchu3OriginalManagerChildTests)

  # Test the translated ORIGINAL 0x57BCB8 actor initialization control
  # flow in all three branches without requiring the unfinished renderer.
  add_executable(Getchu3OriginalManagerInitializerTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_unreal_initializer_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_initializer.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_actor_nav_route.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_actor_type2_target.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_actor_random_triangle.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_actor_ray_geometry.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_game_random.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_spatial_index.cpp")
  target_include_directories(Getchu3OriginalManagerInitializerTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalManagerInitializer57BCB8
    COMMAND Getchu3OriginalManagerInitializerTests)

  add_executable(Getchu3OriginalManagerHostAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_unreal_abi_test.cpp")
  target_include_directories(Getchu3OriginalManagerHostAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalManagerHostAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalManagerInitializerHostAbi
    COMMAND Getchu3OriginalManagerHostAbiTests)

  # Test the actual original 0x57C7CC route-selection control flow
  # through both its native API and source-recovered implementation.
  add_executable(Getchu3OriginalActorRouteHostAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_actor_unreal_route_abi_test.cpp")
  target_include_directories(Getchu3OriginalActorRouteHostAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalActorRouteHostAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalActorRoute57C7CCHostAbi
    COMMAND Getchu3OriginalActorRouteHostAbiTests)

  add_executable(Getchu3OriginalHealthBridgeTests
    "${ORIGINAL_NATIVE_ROOT}/tests/ae3_unreal_health_bridge_test.cpp")
  target_include_directories(Getchu3OriginalHealthBridgeTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalHealthBridgeTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalChangeHPSourceStateReadWrite
    COMMAND Getchu3OriginalHealthBridgeTests)

  add_executable(Getchu3OriginalLuaStageBindingTests
    "${ORIGINAL_NATIVE_ROOT}/tests/lua_runtime_test.cpp"
    ${ORIGINAL_LUA_STAGE_SOURCES})
  target_include_directories(Getchu3OriginalLuaStageBindingTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalLuaStageBindingTests PRIVATE Getchu3Lua50)
  add_test(NAME OriginalLua50HostStageBindings
    COMMAND Getchu3OriginalLuaStageBindingTests)

  if(WIN32)
    add_executable(Getchu3OriginalWindowsSha256Tests
      "${ORIGINAL_NATIVE_ROOT}/tests/windows_original_sha256_test.cpp")
    target_include_directories(Getchu3OriginalWindowsSha256Tests
      PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
    target_link_libraries(Getchu3OriginalWindowsSha256Tests PRIVATE bcrypt)
    add_test(NAME OriginalWindowsSHA256SourceVectors
      COMMAND Getchu3OriginalWindowsSha256Tests)
  endif()
endif()
