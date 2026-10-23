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
  # Verified original controller vtable slot 5: 0x57C620 dispatcher.
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_dispatch_57c620.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_type2_update_57c554.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_route_child_57c27c.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_followup_57c48c.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_routed_update_57c388.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_native_method_router.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_movement_57bf5c.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_vu0_scalar_36f5f4.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_lifecycle_557e30.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_early_36f954.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_tick_bridge_3161b8.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_controller_owner_base.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_polygon_selector_30ffd0.cpp"
  # Original actor navigation initializer 0x57BCB8, source route-edge
  # lookup, type-2 target, exact source RNG / VU0 and 0x70 polygon views.
  # These are real recovered original functions, not actor substitutes.
  "${ORIGINAL_NATIVE_SOURCE}/original_manager_initializer.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_route_follower.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_polygon_raycast.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_polygon_traversal.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_polygon_edge_clip.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_nav_route.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_type2_target.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_random_triangle.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_fallback_sampler.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_fallback_vu0_native.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_fallback_vu0_random.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_actor_target_dispatch.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_vu0_geometry.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_spatial_support.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_spatial_raw_bridge.cpp"
  "${ORIGINAL_NATIVE_SOURCE}/original_stage_spatial_loader.cpp"
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

  # SHA-pinned original 0x30FFD0 incoming polygon adoption,
  # 0x21B788 margin and 0x21D898 nearest fallback.
  add_executable(Getchu3OriginalActorPolygon30FFD0Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_actor_polygon_selector_30ffd0_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_actor_polygon_selector_30ffd0.cpp")
  target_include_directories(Getchu3OriginalActorPolygon30FFD0Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalActorPolygonSelection30FFD0
    COMMAND Getchu3OriginalActorPolygon30FFD0Tests)

  # Complete, independently hash-pinned 0x57C27C child update.
  # The original 0x57BF5C..0x57C554 region is FOUR functions.
  add_executable(Getchu3OriginalRouteChild57C27CTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_route_child_57c27c_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_route_child_57c27c.cpp")
  target_include_directories(Getchu3OriginalRouteChild57C27CTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalRouteChildController57C27C
    COMMAND Getchu3OriginalRouteChild57C27CTests)

  # Original 0x57C388 route-child update: 0x57C7CC owner route,
  # original 0x30C21C child and 0x57C48C next-method installation.
  add_executable(Getchu3OriginalRouted57C388Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_routed_update_57c388_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_routed_update_57c388.cpp")
  target_include_directories(Getchu3OriginalRouted57C388Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerRouted57C388
    COMMAND Getchu3OriginalRouted57C388Tests)

  # Shared original controller object: source 0x57C27C ->
  # 0x57C388 -> 0x57C48C and type-two 0x57C554 use ONE
  # game-owned 0x3161B8 wrapper/global nesting counter.
  add_executable(Getchu3OriginalSharedControllerOwnerTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_controller_owner_base_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_controller_owner_base.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_tick_bridge_3161b8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_native_method_router.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_dispatch_57c620.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_route_child_57c27c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_routed_update_57c388.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_followup_57c48c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_type2_update_57c554.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_movement_57bf5c.cpp")
  target_include_directories(Getchu3OriginalSharedControllerOwnerTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalSharedControllerOwner3161B8
    COMMAND Getchu3OriginalSharedControllerOwnerTests)

  # One real controller-owned adapter for source 0x3161B8 and
  # 0x316174, preserving child-slot identity and global depth.
  add_executable(Getchu3OriginalControllerTickBridgeTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_tick_bridge_3161b8_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_tick_bridge_3161b8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_type2_update_57c554.cpp")
  target_include_directories(Getchu3OriginalControllerTickBridgeTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerTickBridge3161B8
    COMMAND Getchu3OriginalControllerTickBridgeTests)

  # Genuine original 0x316174 virtual update depth/global and
  # 0x3161B8 child+0x10 pointer/status+0x14 lifecycle.
  add_executable(Getchu3OriginalManager316174TickTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_update_316174_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp")
  target_include_directories(Getchu3OriginalManager316174TickTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerUpdate3161743161B8
    COMMAND Getchu3OriginalManager316174TickTests)

  # Actual original 0x36F5F4 scratch-output scalar tail, limited
  # to source caller 0x57C148 / original t0=0; VU0 arithmetic
  # remains an explicitly unrecovered real owner dependency.
  add_executable(Getchu3OriginalMovementScratchTailTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_scratch_tail_36f6a8_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp")
  target_include_directories(Getchu3OriginalMovementScratchTailTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementScratchTail36F6A8
    COMMAND Getchu3OriginalMovementScratchTailTests)

  # Complete actual retail 0x30C57C object construction:
  # 0xD0 allocation, guest parent/vtable fields, 0xB0 source
  # scratch transfer, byte C0 reset and unknown bytes masked.
  add_executable(Getchu3OriginalMovementChild30C57CTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_child_30c57c_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp")
  target_include_directories(Getchu3OriginalMovementChild30C57CTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementChild30C57CSourceCopy
    COMMAND Getchu3OriginalMovementChild30C57CTests)

  # Original 0x36F608..0x36F62C VSUB/VMUL/VADDy/VADDz
  # scalar source reduction; genuine EE registers, never inferred
  # host positions, produce COP1 f1 consumed by scratch status.
  add_executable(Getchu3OriginalMovementVU0ScalarTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_vu0_scalar_36f5f4_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_vu0_scalar_36f5f4.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp")
  target_include_directories(Getchu3OriginalMovementVU0ScalarTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementVU0SourceScalar36F5F4
    COMMAND Getchu3OriginalMovementVU0ScalarTests)

  # Original 0x36F64C..0x36F68C source VU0 vector side effects
  # with genuine midpoint branch and PPACW zero VF7.
  add_executable(Getchu3OriginalMovementVU0VectorFrameTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_vu0_vector_frame_36f5f4_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_vu0_scalar_36f5f4.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp")
  target_include_directories(Getchu3OriginalMovementVU0VectorFrameTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementVU0SourceVectors36F5F4
    COMMAND Getchu3OriginalMovementVU0VectorFrameTests)

  # End-to-end original source vectors -> COP1 f1 -> verified
  # 0x36F6A8 scratch -> fully recovered 0x30C57C child.
  add_executable(Getchu3OriginalMovementVectorChildChainTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_vu0_scratch_child_integration_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_vu0_scalar_36f5f4.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalMovementVectorChildChainTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementVectorScratchChildChain
    COMMAND Getchu3OriginalMovementVectorChildChainTests)

  # Complete source-bounded first-return path with ALL real
  # VF outputs + exact scratch write provenance, then retail
  # 0x30C57C child copy preserving uninitialized source bytes.
  add_executable(Getchu3OriginalMovementFirstReturnKnownTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_first_return_provenance_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_vu0_scalar_36f5f4.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalMovementFirstReturnKnownTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementFirstReturnScratchProvenance
    COMMAND Getchu3OriginalMovementFirstReturnKnownTests)

  # Original real movement-child 0x6B0B20 vtable slot5 0x557E30
  # flag-driven completion and slot1 0x556A0C deletion; preserves
  # original parent+0x14 branch-delay load and 0x3161B8 status.
  add_executable(Getchu3OriginalMovementChildLifecycleTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_child_lifecycle_557e30_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_lifecycle_557e30.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_early_36f954.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp")
  target_include_directories(Getchu3OriginalMovementChildLifecycleTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementChildLifecycle557E30
    COMMAND Getchu3OriginalMovementChildLifecycleTests)

  # Genuine 0x36F954 fast exit in a NORMAL zero-flag child
  # slot5 update: source child+B4 >=1.0 at f12=2.0 ->
  # 0x557E30 status2 -> 0x3161B8 actual child destructor.
  add_executable(Getchu3OriginalNormalChildCompletion36F954Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_child_early_36f954_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_early_36f954.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_lifecycle_557e30.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp")
  target_include_directories(Getchu3OriginalNormalChildCompletion36F954Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalNormalChildCompletion36F954
    COMMAND Getchu3OriginalNormalChildCompletion36F954Tests)

  # Complete source chain: retail VU0 vector inputs, scalar result,
  # source scratch status, actual 30C57C data initialization,
  # zero-flag 557E30 completion and true 3161B8 child deletion.
  add_executable(Getchu3OriginalNormalVUChildChainTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_normal_vu_child_chain_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_vu0_scalar_36f5f4.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_scratch_tail_36f6a8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_early_36f954.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_lifecycle_557e30.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp")
  target_include_directories(Getchu3OriginalNormalVUChildChainTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalNormalMovementVUChildLifecycle
    COMMAND Getchu3OriginalNormalVUChildChainTests)

  # Source-complete 14-instruction original 0x317B9C resource
  # selector: +0x1C0/+0x220/+0x120/+0x140 retail branches.
  add_executable(Getchu3OriginalResourceSelector317B9CTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_resource_selector_317b9c_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp")
  target_include_directories(Getchu3OriginalResourceSelector317B9CTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalManagerResourceSelector317B9C
    COMMAND Getchu3OriginalResourceSelector317B9CTests)

  # Source original 0x557ED8..0x557F30 nonterminal normal child
  # movement setup: real child status reset, original 317B9C
  # resource, genuine 276B68/2767F4/2773D4 and child+C0=1.
  # The game owner must continue original 317B34/36FB18.
  add_executable(Getchu3OriginalNormalBegin557ED8Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_normal_begin_557ed8_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_lifecycle_557e30.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_early_36f954.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp")
  target_include_directories(Getchu3OriginalNormalBegin557ED8Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementNonterminalSetup557ED8
    COMMAND Getchu3OriginalNormalBegin557ED8Tests)

  # The preceding real 0x317B34 resource float source function:
  # parent+0x40==1 -> resource+0x374, or mode low8==0 ->
  # resource+0x3B4. Nonzero-mode VU0 remains unresolved.
  add_executable(Getchu3OriginalResourceFloat317B34Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_resource_float_317b34_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp")
  target_include_directories(Getchu3OriginalResourceFloat317B34Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalManagerResourceFloat317B34
    COMMAND Getchu3OriginalResourceFloat317B34Tests)

  # Original 0x317B34 VU0 VF1 metric under exact PS2 FP subset.
  add_executable(Getchu3OriginalResourceVU0Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_resource_vu0_317b34_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp")
  target_include_directories(Getchu3OriginalResourceVU0Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalManagerResourceVU0Exact317B34
    COMMAND Getchu3OriginalResourceVU0Tests)

  # Original 0x36FB18 completed-status/nonpositive-scale fast exits.
  add_executable(Getchu3OriginalMovementStepFastTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_step_fast_36fb18_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalMovementStepFastTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementStepFast36FB18
    COMMAND Getchu3OriginalMovementStepFastTests)

  # Source 557F34/317B34 metric and original 36FB18 handoff.
  add_executable(Getchu3OriginalMetricPrefixTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_metric_step_557f34_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalMetricPrefixTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementMetricPrefix557F34
    COMMAND Getchu3OriginalMetricPrefixTests)

  # Connected 557E30 -> 557ED8 -> 557F34 -> 36FB18 lifecycle.
  add_executable(Getchu3OriginalMetricLifecycleTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_normal_metric_lifecycle_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_lifecycle_557e30.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_early_36f954.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_metric_step_557f34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_vu0_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_float_317b34.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_step_fast_36fb18.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_normal_begin_557ed8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_resource_selector_317b9c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalMetricLifecycleTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementNormalMetricLifecycle
    COMMAND Getchu3OriginalMetricLifecycleTests)

  # Original 0x36FB5C..0x36FC24 flagged +A8 bit1
  # movement advance, with a separate required 36F8C8 path.
  add_executable(Getchu3OriginalFlaggedMovementTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_flagged_36fb5c_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp")
  target_include_directories(Getchu3OriginalFlaggedMovementTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementFlaggedPass36FB5C
    COMMAND Getchu3OriginalFlaggedMovementTests)

  # Exact original 36FB5C completed-pass commit into typed
  # 30C57C child status+B4 / flags+B8. Fabricated or stale
  # caller return values and 36F8C8 continuations are rejected.
  add_executable(Getchu3OriginalFlaggedChildCommitTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_flagged_child_commit_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_flagged_36fb5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalFlaggedChildCommitTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementFlaggedChildCommit36FB5C
    COMMAND Getchu3OriginalFlaggedChildCommitTests)

  # Genuine source 0x36F8C8 PPACW progress polynomial,
  # source VF1/VF4 outputs and two VU0 ACC stages.
  add_executable(Getchu3OriginalMovementRegenerate36F8C8Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_regenerate_36f8c8_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp")
  target_include_directories(Getchu3OriginalMovementRegenerate36F8C8Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementRegeneration36F8C8
    COMMAND Getchu3OriginalMovementRegenerate36F8C8Tests)

  # Actual original 36FC18 callsite using owned 30C57C child,
  # real source-VU state publisher and original child+B8 flag.
  add_executable(Getchu3OriginalMovementRegenerateOwnedTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_movement_regenerate_owned_36f8c8_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_owned_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_regenerate_36f8c8.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_movement_child_30c57c.cpp")
  target_include_directories(Getchu3OriginalMovementRegenerateOwnedTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementRegenerationOwned36F8C8
    COMMAND Getchu3OriginalMovementRegenerateOwnedTests)

  # Original 0x57BF5C source movement controller. Native branch
  # sequencing requires explicit real 0x21B854/0x36F5F4/VU0 and
  # source actor child callbacks; never invent missing movements.
  add_executable(Getchu3OriginalMovement57BF5CTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_movement_57bf5c_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_movement_57bf5c.cpp")
  target_include_directories(Getchu3OriginalMovement57BF5CTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalMovementController57BF5C
    COMMAND Getchu3OriginalMovement57BF5CTests)

  # Original 0x57C620 vtable dispatch -> real native original
  # method transitions 0x57C27C -> 0x57C388 -> 0x57C48C.
  # Unknown methods/source this-offsets remain actual owner calls.
  add_executable(Getchu3OriginalMethodDispatchIntegrationTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_native_method_router_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_native_method_router.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_movement_57bf5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_update_316174.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_dispatch_57c620.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_route_child_57c27c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_routed_update_57c388.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_followup_57c48c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_type2_update_57c554.cpp")
  target_include_directories(Getchu3OriginalMethodDispatchIntegrationTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalRecoveredControllerMethodDispatchIntegration
    COMMAND Getchu3OriginalMethodDispatchIntegrationTests)

  # Distinct original 0x57C48C controller method, installed by
  # source 0x57C388. Complete 50-instruction original behavior.
  add_executable(Getchu3OriginalFollowup57C48CTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_followup_57c48c_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_followup_57c48c.cpp")
  target_include_directories(Getchu3OriginalFollowup57C48CTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerFollowup57C48C
    COMMAND Getchu3OriginalFollowup57C48CTests)

  # Recovered exact four-lane PCEQW packed SQ equality at
  # 0x57C1E0, 0x57C508 and 0x57C5D4. No float comparison or
  # guessed missing +0x20/+0x40 source operand is permitted.
  add_executable(Getchu3OriginalPackedPceqwTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_packed_compare_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_movement_57bf5c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_followup_57c48c.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_type2_update_57c554.cpp")
  target_include_directories(Getchu3OriginalPackedPceqwTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerPackedPCEQW128Bit
    COMMAND Getchu3OriginalPackedPceqwTests)

  # Original action-type-2 0x57C554 owner child lifecycle/dispatch.
  add_executable(Getchu3OriginalType2Controller57C554Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_type2_update_57c554_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_type2_update_57c554.cpp")
  target_include_directories(Getchu3OriginalType2Controller57C554Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerType2Update57C554
    COMMAND Getchu3OriginalType2Controller57C554Tests)

  # Exact original USA vtable entry 0x6B1834 -> 0x57C620,
  # native direct/virtual update dispatch and gated source cleanup.
  add_executable(Getchu3OriginalManagerDispatch57C620Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_manager_dispatch_57c620_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_dispatch_57c620.cpp")
  target_include_directories(Getchu3OriginalManagerDispatch57C620Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalControllerDispatch57C620
    COMMAND Getchu3OriginalManagerDispatch57C620Tests)

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
    "${ORIGINAL_NATIVE_SOURCE}/original_manager_child.cpp"
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

  # Reuse the recovered 0x21D898 grid-hit + 0x21DA2C full-registry
  # original VU0 selectors directly from the native Unreal bridge.
  add_executable(Getchu3OriginalStageNearestHostAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_stage_nearest_unreal_abi_test.cpp")
  target_include_directories(Getchu3OriginalStageNearestHostAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalStageNearestHostAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalLoadedStageSpatialNearestHostAbi
    COMMAND Getchu3OriginalStageNearestHostAbiTests)

  add_executable(Getchu3OriginalRawPolygonNearestHostAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_polygon_nearest_original_data_abi_test.cpp")
  target_include_directories(Getchu3OriginalRawPolygonNearestHostAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalRawPolygonNearestHostAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalRawSpatialHeaderGridNearestHostAbi
    COMMAND Getchu3OriginalRawPolygonNearestHostAbiTests)

  add_executable(Getchu3OriginalPolygonNearestHostAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_polygon_nearest_unreal_abi_test.cpp")
  target_include_directories(Getchu3OriginalPolygonNearestHostAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalPolygonNearestHostAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalPolygonNearest21D89821DA2CHostAbi
    COMMAND Getchu3OriginalPolygonNearestHostAbiTests)

  add_executable(Getchu3OriginalPolygonRayHostAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_polygon_unreal_raycast_abi_test.cpp")
  target_include_directories(Getchu3OriginalPolygonRayHostAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalPolygonRayHostAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalPolygonRaycast4C1B3CHostAbi
    COMMAND Getchu3OriginalPolygonRayHostAbiTests)

  add_executable(Getchu3OriginalActorRouteRaycastV2Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_actor_unreal_route_raycast_v2_test.cpp")
  target_include_directories(Getchu3OriginalActorRouteRaycastV2Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalActorRouteRaycastV2Tests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalRoute57C7CCNativeRaycast4C1B3C
    COMMAND Getchu3OriginalActorRouteRaycastV2Tests)

  # Independent genuine 0x57CB4C/0x57CC48 VU0 instruction
  # regression with scalar oracle; does NOT need Lua or Unreal.
  add_executable(Getchu3OriginalFallbackNativeVu0Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_actor_fallback_vu0_native_test.cpp"
    "${ORIGINAL_NATIVE_SOURCE}/original_actor_fallback_vu0_native.cpp")
  target_include_directories(Getchu3OriginalFallbackNativeVu0Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  add_test(NAME OriginalFallbackVU0FixedInstructionVectors
    COMMAND Getchu3OriginalFallbackNativeVu0Tests)

  add_executable(Getchu3OriginalActorTargetAbiTests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_actor_target_unreal_abi_test.cpp")
  target_include_directories(Getchu3OriginalActorTargetAbiTests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalActorTargetAbiTests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalActorTarget57C8ECNativeFastAndRandom
    COMMAND Getchu3OriginalActorTargetAbiTests)

  add_executable(Getchu3OriginalRouteNativeTargetV3Tests
    "${ORIGINAL_NATIVE_ROOT}/tests/original_route_native_target_v3_test.cpp")
  target_include_directories(Getchu3OriginalRouteNativeTargetV3Tests
    PRIVATE "${ORIGINAL_NATIVE_INCLUDE}")
  target_link_libraries(Getchu3OriginalRouteNativeTargetV3Tests
    PRIVATE Getchu3OriginalLuaBridge)
  add_test(NAME OriginalRoute57C7CCTarget57C8ECNativeCommit
    COMMAND Getchu3OriginalRouteNativeTargetV3Tests)

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
