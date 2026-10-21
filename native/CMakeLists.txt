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

# cgltf is a MIT single-file glTF parser, pinned to an immutable upstream
# commit. It parses host-native asset data, never console GPU commands.
FetchContent_Declare(GetchuCgltf
  URL "https://codeload.github.com/jkuhlmann/cgltf/tar.gz/85cd62382dfea638278962690cf515023f33ed00"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(GetchuCgltf)
if(NOT getchucgltf_POPULATED)
  FetchContent_Populate(GetchuCgltf)
endif()
add_library(Getchu3Assets STATIC src/archive.cpp src/tim2.cpp src/stage_scripts.cpp src/player_state.cpp src/lua_stage.cpp src/controller_timeline.cpp src/gltf_scene.cpp src/pck_archive.cpp src/level_scene.cpp src/i3m_animation.cpp src/native_walking.cpp src/i3c_collision.cpp src/native_skinning.cpp src/actor_world.cpp src/retail_lua50.cpp src/original_player_mode.cpp src/original_monkey_accessors.cpp src/original_retail_call_chains.cpp src/original_attachment_engine.cpp src/original_attachment_followup.cpp src/original_action_runtime.cpp src/original_player_factory.cpp src/original_manager_child.cpp src/original_spatial_index.cpp src/original_spatial_support.cpp src/original_vu0_geometry.cpp src/original_spatial_raw_bridge.cpp src/original_stage_spatial_loader.cpp)
target_include_directories(Getchu3Assets PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(Getchu3Assets PRIVATE "${getchucgltf_SOURCE_DIR}")
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
                          user32 gdi32 comdlg32 shell32 windowscodecs ole32)
    target_compile_definitions(Getchu3Native PRIVATE UNICODE _UNICODE)
    if(MSVC)
        target_compile_options(Getchu3Native PRIVATE /W4 /permissive-)
    endif()
endif()

enable_testing()
# Authentic SCUS_975.01 secondary Player virtual method, reconstructed in C++.
add_executable(Getchu3OriginalPlayerModeTests tests/original_player_mode_test.cpp)
target_link_libraries(Getchu3OriginalPlayerModeTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailPlayerVirtualActionDispatch COMMAND Getchu3OriginalPlayerModeTests)
add_executable(Getchu3OriginalChipCounterTests tests/original_chip_counter_test.cpp)
target_link_libraries(Getchu3OriginalChipCounterTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailChipCounterGainAndWrap COMMAND Getchu3OriginalChipCounterTests)
add_executable(Getchu3OriginalChipStateTests tests/original_chip_state_write_test.cpp)
target_link_libraries(Getchu3OriginalChipStateTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailChipCounterExactMMIDataWrites COMMAND Getchu3OriginalChipStateTests)
add_executable(Getchu3OriginalMonkeyAccessorTests tests/original_monkey_accessors_test.cpp)
target_link_libraries(Getchu3OriginalMonkeyAccessorTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailMonkeyAuxiliaryVirtualMethods COMMAND Getchu3OriginalMonkeyAccessorTests)
add_executable(Getchu3OriginalActorLifecycleTests tests/original_retail_call_chains_test.cpp)
target_link_libraries(Getchu3OriginalActorLifecycleTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailActorAndChipLifecycleCallChains COMMAND Getchu3OriginalActorLifecycleTests)
add_executable(Getchu3OriginalAttachmentEngineTests tests/original_attachment_engine_test.cpp)
target_link_libraries(Getchu3OriginalAttachmentEngineTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailMonkeyAttachmentConstructorsAndHandlers COMMAND Getchu3OriginalAttachmentEngineTests)
add_executable(Getchu3OriginalAttachmentFollowupTests tests/original_attachment_followup_test.cpp)
target_link_libraries(Getchu3OriginalAttachmentFollowupTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailAttachmentTimerAndVirtualDispatch COMMAND Getchu3OriginalAttachmentFollowupTests)
add_executable(Getchu3OriginalActionRuntimeTests tests/original_action_runtime_test.cpp)
target_link_libraries(Getchu3OriginalActionRuntimeTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailActionReadinessAndManagerInit COMMAND Getchu3OriginalActionRuntimeTests)
add_executable(Getchu3OriginalPlayerFactoryTests tests/original_player_factory_test.cpp)
target_link_libraries(Getchu3OriginalPlayerFactoryTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailPlayerResourceLookupAndManagerDependency COMMAND Getchu3OriginalPlayerFactoryTests)
add_executable(Getchu3OriginalManagerChildTests tests/original_manager_child_test.cpp)
target_link_libraries(Getchu3OriginalManagerChildTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetail112ByteManagerChildConstructionAndVMethods COMMAND Getchu3OriginalManagerChildTests)
add_executable(Getchu3OriginalSpatialRegistryTests tests/original_spatial_index_test.cpp)
target_link_libraries(Getchu3OriginalSpatialRegistryTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailNearestSelectionAndRegistryTeardown COMMAND Getchu3OriginalSpatialRegistryTests)
add_executable(Getchu3OriginalSpatialSupportTests tests/original_spatial_support_test.cpp)
target_link_libraries(Getchu3OriginalSpatialSupportTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailGridPlanePolylineAndChildDestructor COMMAND Getchu3OriginalSpatialSupportTests)
add_executable(Getchu3OriginalVU0GeometryTests tests/original_vu0_geometry_test.cpp)
target_link_libraries(Getchu3OriginalVU0GeometryTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailVU0PolygonMeanXYZDistanceAndNearest COMMAND Getchu3OriginalVU0GeometryTests)
add_executable(Getchu3OriginalSpatialRawBridgeTests tests/original_spatial_raw_bridge_test.cpp)
target_link_libraries(Getchu3OriginalSpatialRawBridgeTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailRawRecordVerticesAndLoadedHeaderResolution COMMAND Getchu3OriginalSpatialRawBridgeTests)
add_executable(Getchu3OriginalStageSpatialLoaderTests tests/original_stage_spatial_loader_test.cpp)
target_link_libraries(Getchu3OriginalStageSpatialLoaderTests PRIVATE Getchu3Assets)
add_test(NAME OriginalRetailStageSourceToSpatialRegistry COMMAND Getchu3OriginalStageSpatialLoaderTests)
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
add_test(NAME NativeLua50StageRunnerDemo
  COMMAND Getchu3LuaStageRunner
          "${CMAKE_CURRENT_SOURCE_DIR}/examples/native_binding_demo.lua" "entry")
set_tests_properties(NativeLua50StageRunnerDemo PROPERTIES
  PASS_REGULAR_EXPRESSION "Native entities declared: 4")


add_executable(Getchu3ControllerTimelineTests tests/controller_timeline_test.cpp)
target_link_libraries(Getchu3ControllerTimelineTests PRIVATE Getchu3Assets)
add_test(NAME NativeLua50TimedControllerActions COMMAND Getchu3ControllerTimelineTests)

add_executable(Getchu3GltfSceneTests tests/gltf_scene_test.cpp)
target_link_libraries(Getchu3GltfSceneTests PRIVATE Getchu3Assets)
add_test(NAME NativeGlbSceneMeshNodesAndBoundingBox COMMAND Getchu3GltfSceneTests)

add_executable(Getchu3GlbSceneProbe src/glb_scene_probe.cpp)
target_link_libraries(Getchu3GlbSceneProbe PRIVATE Getchu3Assets)

add_executable(Getchu3PckProbe src/pck_probe.cpp)
target_link_libraries(Getchu3PckProbe PRIVATE Getchu3Assets)
add_executable(Getchu3PckTests tests/pck_archive_test.cpp)
target_link_libraries(Getchu3PckTests PRIVATE Getchu3Assets)
add_test(NAME NativeOriginalAE3PCKMembersAndNames COMMAND Getchu3PckTests)
add_executable(Getchu3PlacementTests tests/level_placement_test.cpp)
target_link_libraries(Getchu3PlacementTests PRIVATE Getchu3Assets)
add_test(NAME NativeOriginalLevelPLCPlacement COMMAND Getchu3PlacementTests)
add_executable(Getchu3LevelSceneProbe src/level_scene_probe.cpp)
target_link_libraries(Getchu3LevelSceneProbe PRIVATE Getchu3Assets)

add_executable(Getchu3I3mProbe src/i3m_probe.cpp)
target_link_libraries(Getchu3I3mProbe PRIVATE Getchu3Assets)
add_executable(Getchu3I3mTests tests/i3m_animation_test.cpp)
target_link_libraries(Getchu3I3mTests PRIVATE Getchu3Assets)
add_test(NAME NativeOriginalI3MQuaternionAnimation COMMAND Getchu3I3mTests)

add_executable(Getchu3MovementTests tests/native_walking_test.cpp)
target_link_libraries(Getchu3MovementTests PRIVATE Getchu3Assets)
add_test(NAME NativePlayerWalkJumpOnOriginalMeshTriangles COMMAND Getchu3MovementTests)

add_executable(Getchu3I3cProbe src/i3c_probe.cpp)
target_link_libraries(Getchu3I3cProbe PRIVATE Getchu3Assets)
add_executable(Getchu3I3cTests tests/i3c_collision_test.cpp)
target_link_libraries(Getchu3I3cTests PRIVATE Getchu3Assets)
add_test(NAME NativeRealI3cGeometryWalkingAndBVH COMMAND Getchu3I3cTests)

add_executable(Getchu3WallMotionTests tests/wall_collision_test.cpp)
target_link_libraries(Getchu3WallMotionTests PRIVATE Getchu3Assets)
add_test(NAME NativeI3cWallSlidingAndSpawn COMMAND Getchu3WallMotionTests)

add_executable(Getchu3NativeSkinningTests tests/native_skinning_test.cpp)
target_link_libraries(Getchu3NativeSkinningTests PRIVATE Getchu3Assets)
add_test(NAME NativeOriginalI3mGltfSkinnedBoneAnimation COMMAND Getchu3NativeSkinningTests)

add_executable(Getchu3SkinningProbe src/native_skinning_probe.cpp)
target_link_libraries(Getchu3SkinningProbe PRIVATE Getchu3Assets)

add_executable(Getchu3ActorWorldTests tests/actor_world_test.cpp)
target_link_libraries(Getchu3ActorWorldTests PRIVATE Getchu3Assets)
add_test(NAME NativeOriginalLuaEntitiesToAnimatedSceneActors COMMAND Getchu3ActorWorldTests)

add_executable(Getchu3ActorWorldProbe src/actor_world_probe.cpp)
target_link_libraries(Getchu3ActorWorldProbe PRIVATE Getchu3Assets)

add_executable(Getchu3RetailLua50Tests tests/retail_lua50_test.cpp)
target_link_libraries(Getchu3RetailLua50Tests PRIVATE Getchu3Assets)
add_test(NAME NativeActualRetailLua50FormatRetarget COMMAND Getchu3RetailLua50Tests)

# Audit actual game-owned ISO stages, without synthetic scenery or emulated hardware.
add_executable(Getchu3RetailStageAudit src/retail_stage_audit.cpp)
target_link_libraries(Getchu3RetailStageAudit PRIVATE Getchu3Assets)
