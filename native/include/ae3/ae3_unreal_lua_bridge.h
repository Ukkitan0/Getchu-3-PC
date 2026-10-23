#pragma once
/* C ABI for the existing authentic Ape Escape 3 Lua 5.0 native runtime.
 *
 * Source: native/src/lua_stage.cpp and native/src/retail_lua50.cpp.
 * This is NOT a Lua replacement, MIPS interpreter, or substitute engine.
 *
 * All data bytes belong to the owner and stay local to the process. Host
 * callbacks MUST implement original source-backed game manager behavior.
 * Reject a callback until that behavior is available; never acknowledge
 * synthetic actors, no-op flags or invented stage parameters as success.
 */
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(AE3_LUA_BRIDGE_BUILD)
#    define AE3_LUA_EXPORT __declspec(dllexport)
#  else
#    define AE3_LUA_EXPORT __declspec(dllimport)
#  endif
#else
#  define AE3_LUA_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AE3_LUA_BRIDGE_API_VERSION 5u

typedef struct Ae3LuaRuntime Ae3LuaRuntime;

/* Original Player virtual slot 2 at retail VA 0x34A014.
 * These borrowed pointers MUST refer to genuine source-recovered Player
 * subobjects. No allocation, fallback instance or guessed component occurs.
 * The secondary factory pointer must refer to the genuine original owner;
 * null preserves the original feature gate for the last six entries.
 */
typedef struct Ae3OriginalPlayerResourcePointers {
    void* player_at_5c;
    void* main_player_at_60;
    void* item_chatcher_at_64;
    void* force_at_180;
    void* bar_interface_at_70;
    void* iZone_at_110;
    void* iHanger_at_188;
    void* iRope_at_1a0;
    void* iDashBall_at_1d0;
    void* iHeroSwitch_at_258;
} Ae3OriginalPlayerResourcePointers;

/* key16 is exactly sixteen original bytes (zero padded where required).
 * Returns a borrowed genuine original subobject pointer or null, directly
 * using the recovered source function originalPlayerResourceLookup_34a014.
 */
AE3_LUA_EXPORT void* ae3_original_player_resource_lookup(
    const Ae3OriginalPlayerResourcePointers* pointers,
    const uint8_t* original_key16, void* original_secondary_factory);

/* Original actor controller initializer at SCUS_975.01 VA 0x57BCB8.
 * Records are the exact 0x70-byte original stage spatial records; route
 * polygon indices and navigation bytes must be taken from owner-source data.
 * All native allocations and RNG lifetime belong to this native session.
 * This invokes recovered source math, NOT a fabricated Unreal Actor spawn.
 */
typedef struct Ae3OriginalManagerInitSession Ae3OriginalManagerInitSession;
typedef struct Ae3OriginalVec4 {
    float x,y,z,w;
} Ae3OriginalVec4;
typedef struct Ae3OriginalManagerInitStateV1 {
    uint32_t source_method08, source_method0c;
    uint8_t source_flag1c, source_alternative30;
    uint16_t reserved;
    Ae3OriginalVec4 controller20, target40, midpoint60;
    uint32_t target_polygon_index; /* UINT32_MAX if no target. */
} Ae3OriginalManagerInitStateV1;
typedef struct Ae3OriginalManagerInitSourceV1 {
    const void* original_records70;
    size_t original_record_count;
    uint32_t original_actor_polygon_index;
    const uint32_t* original_route_indices;
    size_t original_route_count;
    const void* original_navigation_bytes;
    size_t original_navigation_size;
    Ae3OriginalVec4 original_owner_position40;
    Ae3OriginalVec4 original_owner_direction50;
} Ae3OriginalManagerInitSourceV1;
typedef struct Ae3OriginalManagerInitCallsV1 {
    void* user;
    /* Original unresolved 0x21B854 must provide a real projected vector. */
    int (*project_original_21b854)(
        void* user, const void* original_polygon70,
        const Ae3OriginalVec4* start, const Ae3OriginalVec4* end,
        Ae3OriginalVec4* projected);
    /* Original unresolved 0x57C7CC must advance real actor/route owner. */
    int (*continue_original_57c7cc)(
        void* user, const Ae3OriginalVec4* original_midpoint);
} Ae3OriginalManagerInitCallsV1;

AE3_LUA_EXPORT Ae3OriginalManagerInitSession*
ae3_original_manager_init_create(uint32_t original_game_seed);
AE3_LUA_EXPORT int ae3_original_manager_init_destroy(
    Ae3OriginalManagerInitSession* session);
/* Return original branch 1 near-owner, 2 kind-two, 3 general projection;
 * 0 on missing owner, malformed source, unresolved callback or exception.
 * output state is written ONLY on success and retains no source pointers.
 * Callers must atomically roll back original manager effects on failure.
 */
AE3_LUA_EXPORT int ae3_original_manager_init_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* original,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV1* real_calls);
/* Version 2 runs recovered 0x57C7CC internally, using its exact original
 * target/route branch logic. Only the independently original-coded
 * 0x21B854, 0x4C1B3C and 0x57C8EC owner operations remain callbacks.
 */
typedef struct Ae3OriginalManagerInitCallsV2 {
    void* user;
    int (*project_original_21b854)(
        void* user, const void* original_polygon70,
        const Ae3OriginalVec4* start, const Ae3OriginalVec4* end,
        Ae3OriginalVec4* projected);
    int (*original_project_4c1b3c)(
        void* user, uint32_t actor_polygon, uint32_t* projected_polygon);
    int (*original_apply_target_57c8ec)(
        void* user, uint32_t target_polygon);
} Ae3OriginalManagerInitCallsV2;
AE3_LUA_EXPORT int ae3_original_manager_init_step_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* original,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV2* original_calls);
AE3_LUA_EXPORT const char* ae3_original_manager_init_last_error(
    const Ae3OriginalManagerInitSession* session);

/* Recovered original 0x57C8EC controller target-vector dispatch.
 * Variant !=0: original 0x21B788 polygon eligibility and source low64
 * controller pass-through, only if polygon accepts source point.
 * Variant ==0: original 0x57C98C three-vertex VU0 weighted random
 * selection using the PERSISTENT source 0x36F3C0 game RNG.
 * The unrecovered 0x57CA68 fallback VU0 transform is rejected.
 * point_count comes from genuine original 0x1E-byte stage descriptor;
 * 0x70 record bytes must come from the original 0x21D46C stage loader.
 * A successful result owns ONLY original $v0 low64 bits.
 */
typedef struct Ae3OriginalActorTargetRequestV1 {
    const void* original_record70;
    uint16_t original_point_count;
    uint16_t reserved;
    int original_variant30;
    Ae3OriginalVec4 original_controller20;
} Ae3OriginalActorTargetRequestV1;
typedef struct Ae3OriginalActorTargetResultV1 {
    uint64_t original_low64;
    int original_branch; /* 1 = passthrough, 2 = triangle random. */
} Ae3OriginalActorTargetResultV1;
AE3_LUA_EXPORT int ae3_original_actor_target_57c8ec(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorTargetRequestV1* source,
    Ae3OriginalActorTargetResultV1* out);
/* The first source target API is low64-only. Original 0x57C8EC's real
 * QMFC2 result is 128 bits: V1 will now reject any nonzero upper half
 * instead of silently corrupting actor steering. Prefer V2 below.
 * The source 0x57CA68 fixed VU0 candidate transform now exists.
 */
typedef struct Ae3OriginalActorTargetRequestV2 {
    Ae3OriginalActorTargetRequestV1 original;
    Ae3OriginalVec4 original_owner14_vector40;
    uint32_t original_q_bits;
    uint32_t original_vu0_r_bits;
    int original_fallback_operands_available;
} Ae3OriginalActorTargetRequestV2;
typedef struct Ae3OriginalActorTargetResultV2 {
    uint64_t original_low64;
    uint64_t original_high64;
    int original_branch; /* 1 = pass-through, 2 = triangle, 3 = fallback */
    uint32_t original_vu0_r_bits;
} Ae3OriginalActorTargetResultV2;
AE3_LUA_EXPORT int ae3_original_actor_target_57c8ec_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorTargetRequestV2* source,
    Ae3OriginalActorTargetResultV2* out);

/* Fully recovered original polygon projection chain:
 * 0x4C1B3C ray preparation -> 0x21BD3C traversal -> 0x21EE00 edge clip.
 * Original stage loader must provide 0x70-byte decoded records AND their
 * genuine navigation adjacency. No invented graph or Unreal raycast.
 * A projected_polygon of UINT32_MAX means no accepted source polygon.
 */
typedef struct Ae3OriginalPolygonRaySourceV1 {
    const void* original_records70;
    size_t original_record_count;
    const uint8_t* original_degree;
    const uint32_t* original_neighbor3;
    const uint16_t* original_action_kind3;
    uint32_t original_start_polygon;
    Ae3OriginalVec4 original_ray_start;
    Ae3OriginalVec4 original_ray_end;
    uint32_t original_max_recursions;
} Ae3OriginalPolygonRaySourceV1;
typedef struct Ae3OriginalPolygonRayResultV1 {
    uint32_t last_original_polygon;
    uint32_t original_polygons_tested;
    uint32_t original_edge_tests;
    uint32_t original_zero_kind_recursions;
    int original_hit_point_available;
    Ae3OriginalVec4 original_hit_point;
} Ae3OriginalPolygonRayResultV1;
/* Source raycast through exact original math, no host-supplied result.
 * Shares initializer session error/lifetime guard. Output untouched on
 * failure. Valid source polygon IDs and 1..4096 recursion budget required.
 */
AE3_LUA_EXPORT int ae3_original_polygon_project_4c1b3c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalPolygonRaySourceV1* original,
    Ae3OriginalPolygonRayResultV1* output);

/* Source recovered 0x57C7CC original route traversal. This is the real
 * source actor's route control flow, not a newly invented path follower.
 * Polygon IDs and 0x70-byte records belong to the source-decoded stage.
 * UINT32_MAX is the original missing polygon sentinel at the host ABI.
 * Calls to unavailable original 0x4C1B3C/0x57C8EC owner operations reject.
 */
typedef struct Ae3OriginalRouteSourceV1 {
    const void* original_records70;
    size_t original_record_count;
    uint32_t actor_polygon_index;
    uint32_t active_target_polygon_index;
    const uint32_t* original_route_indices;
    size_t original_route_count;
    int original_has_multiple_polygons1c;
} Ae3OriginalRouteSourceV1;
typedef struct Ae3OriginalRouteCallsV1 {
    void* user;
    /* Genuine original projection must return a valid source polygon ID
     * or UINT32_MAX for no hit; returning zero rejects the original call.
     */
    int (*original_project_4c1b3c)(void* user,
        uint32_t source_actor_polygon, uint32_t* projected_polygon);
    /* Genuine original 0x57C8EC target update, 1 only after performed. */
    int (*original_apply_target_57c8ec)(void* user,
        uint32_t target_polygon);
} Ae3OriginalRouteCallsV1;
typedef struct Ae3OriginalRouteResultV1 {
    uint32_t active_target_polygon_index;
    uint32_t original_targets_applied;
    uint32_t original_projections_performed;
    uint32_t original_following_indices;
    int original_early_current_target_match;
} Ae3OriginalRouteResultV1;
/* Uses the same native session guard as source initializer, forbidding
 * reentrant route operations and protecting real game manager ownership.
 * On success, output reflects exact original 0x57C7CC branch behavior.
 */
AE3_LUA_EXPORT int ae3_original_actor_route_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV1* original,
    const Ae3OriginalRouteCallsV1* genuine_manager,
    Ae3OriginalRouteResultV1* output);

/* Version 2: actually execute original 0x4C1B3C projection inside the
 * recovered 0x57C7CC follower, using original 0x21BD3C adjacency and
 * 0x21EE00 edge intersection. The real 0x57C8EC owner must return the
 * exact LOW 64 original target-vector bits produced by its routine.
 * Initial projection uses controller+0x40; subsequent projections use
 * DMFC2 source low 64 bits with zero high 64, as original 0x57C894.
 */
typedef struct Ae3OriginalRouteSourceV2 {
    Ae3OriginalRouteSourceV1 original;
    const uint8_t* original_degree;
    const uint32_t* original_neighbor3;
    const uint16_t* original_action_kind3;
    Ae3OriginalVec4 original_incoming_a1;
    Ae3OriginalVec4 original_controller40;
    uint32_t original_max_recursions;
} Ae3OriginalRouteSourceV2;
typedef struct Ae3OriginalRouteCallsV2 {
    void* user;
    int (*original_apply_target_57c8ec)(
        void* user, uint32_t original_polygon,
        uint64_t* original_target_low64);
} Ae3OriginalRouteCallsV2;
AE3_LUA_EXPORT int ae3_original_actor_route_step_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV2* original,
    const Ae3OriginalRouteCallsV2* actual_target_manager,
    Ae3OriginalRouteResultV1* output);

/* Version 3: no guessed source target vectors. Call the native
 * originalActorTargetVector_57c8ec implementation for each route update,
 * including original 0x21B788 eligibility and 0x57C98C game RNG.
 * point_counts[] MUST match original 0x1E-byte stage descriptor counts.
 * Owner commits computed source low64 to its real actor controller.
 * Reject if original 0x57CA68 fallback VU0 is required or owner refuses.
 * Return failure without advancing shared original RNG or publishing output.
 */
typedef struct Ae3OriginalRouteSourceV3 {
    Ae3OriginalRouteSourceV2 original;
    const uint16_t* original_polygon_point_counts;
    Ae3OriginalVec4 original_controller20;
    int original_controller_variant30;
} Ae3OriginalRouteSourceV3;
typedef struct Ae3OriginalRouteCallsV3 {
    void* user;
    /* BEGIN, stage individual updates, COMMIT or ABORT as one actual
     * actor-owner transaction. Manager must not expose partial updates.
     * Missing transaction callbacks reject original actor route.
     */
    int (*begin_original_actor_update)(void* user);
    int (*stage_original_target_57c8ec)(
        void* user, uint32_t original_polygon,
        uint64_t original_target_low64);
    int (*commit_original_actor_update)(void* user);
    void (*abort_original_actor_update)(void* user);
} Ae3OriginalRouteCallsV3;
AE3_LUA_EXPORT int ae3_original_actor_route_step_v3(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV3* source,
    const Ae3OriginalRouteCallsV3* real_actor_manager,
    Ae3OriginalRouteResultV1* result);




/* Every user string is copied into this library; callbacks must not keep the
 * borrowed pointers past the call. Return 1 only after the real manager
 * successfully accepts the operation. Zero rejects the operation and makes
 * original Lua stop with a genuine binding error.
 *
 * Entity class/name/tag and flag scope/key are supplied as exact byte hex.
 * Lua field JSON uses a LOSSLESS typed representation:
 *  nil: {"kind":"nil"}
 *  boolean: {"kind":"boolean","value":true}
 *  number: {"kind":"number","value":1.5}
 *  text: {"kind":"text","hex":"..."} (exact original string bytes)
 *  array: {"kind":"array","items":[...]}
 *  object: {"kind":"object","entries":[{"key_hex":"...","value":...}]}
 * This is not a guessed gameplay schema. No source bytes are lost by Unicode
 * transcoding, embedded NUL, or JSON key normalization.
 */
typedef struct Ae3LuaManagerV5 {
    uint32_t api_version;
    uint32_t struct_size;
    void* user;

    int (*create_entity)(void*, const char* original_class_hex,
                         const char* original_name_hex, const char* original_tag_hex,
                         const char* original_fields_json,
                         const char* original_controllers_json);
    int (*submit_parameters)(void*, const char* original_parameters_json);
    int (*write_flag)(void*, const char* scope_hex, const char* key_hex, int value);
    /* 1 means known, 0 means original manager data unavailable. */
    int (*read_flag)(void*, const char* scope_hex, const char* key_hex, int* value);
    int (*read_stage_int)(void*, const char* typed_args_json, int* value);
    int (*read_survival)(void*, int* value);
    int (*select_weapon)(void*, int original_selection);
    /* Returns 1 only when 1033 original source bytes were provided. */
    int (*read_original_player_progress)(void*, void* destination, size_t exact_size);
    /* 1033 bytes; manager MUST atomically commit source-backed player state.
     * Return 0 if player manager/write ownership is missing or cannot commit.
     * The native bridge will NEVER synthesize or cache a successful write.
     */
    int (*write_original_player_progress)(void*, const void* source, size_t exact_size);
    /* ONLY source-recovered, verified original-ELF void-return Lua callbacks.
     * original_name is an ASCII Lua global, native address is an independently
     * verified retail function VA (NOT a CreateBind JAL site).
     * typed_arguments_json preserves source Lua byte values. Return 1 only
     * after the original native behavior has actually been performed.
     */
    int (*invoke_original_void)(void*, uint32_t original_va,
                                const char* original_name,
                                const char* typed_arguments_json);
} Ae3LuaManagerV5;

/* All API functions return 1 on success, 0 on failure.
 * On failure ae3_lua_last_error provides the error text while the handle
 * remains alive. All operations and callback invocations are synchronous
 * and must run on the Unreal game thread. Callbacks MUST NOT re-enter.
 */
AE3_LUA_EXPORT uint32_t ae3_lua_api_version(void);
AE3_LUA_EXPORT Ae3LuaRuntime* ae3_lua_create(const Ae3LuaManagerV5* manager);
/* Returns 0 without destroying if Lua or a manager callback is executing.
 * Callers MUST NOT unload the host DLL or discard the handle after 0.
 */
AE3_LUA_EXPORT int ae3_lua_destroy(Ae3LuaRuntime* runtime);
AE3_LUA_EXPORT int ae3_lua_reset(Ae3LuaRuntime* runtime);
/* Register one *actually implemented* original source Lua function before
 * executing its stage chunk. Registration requires an original verified VA,
 * a valid global name, and a live invoke_original_void manager. Never
 * auto-register functions merely because a name appears in the ELF audit.
 * Call fails on duplicate or built-in globals, invalid VA, or after execution.
 */
AE3_LUA_EXPORT int ae3_lua_register_original_void(
    Ae3LuaRuntime* runtime, const char* original_name, uint32_t original_va);
AE3_LUA_EXPORT int ae3_lua_execute_original_chunk(
    Ae3LuaRuntime* runtime, const void* bytes, size_t length,
    const char* original_chunk_path);
AE3_LUA_EXPORT int ae3_lua_setup_area(Ae3LuaRuntime* runtime,
                                       const char* original_spawn_tag);
/* Native port of the original ChangeHP delta-and-clamp routine.
 * Reuses recovered 1033-byte PlayerProgress and refuses mutations unless
 * the real host provides both read and atomic write managers. On success,
 * out_health is the committed original player_life, not a simulated value.
 */
AE3_LUA_EXPORT int ae3_lua_apply_original_health_delta(
    Ae3LuaRuntime* runtime, float delta, float* out_health);
AE3_LUA_EXPORT const char* ae3_lua_last_error(const Ae3LuaRuntime* runtime);
AE3_LUA_EXPORT size_t ae3_lua_declared_entity_count(const Ae3LuaRuntime* runtime);
AE3_LUA_EXPORT size_t ae3_lua_parameter_count(const Ae3LuaRuntime* runtime);

#ifdef __cplusplus
}
#endif
